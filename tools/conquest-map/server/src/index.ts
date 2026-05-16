import "dotenv/config";
import express from "express";
import cors from "cors";
import mysql from "mysql2/promise";
import { nearestZone, STATIC_ZONES, zoneName } from "./static-zones.js";

const PORT = Number(process.env.PORT ?? 4317);
const BANNER_ENTRY = Number(process.env.BANNER_ENTRY ?? 400010);
const WAYPOINT_ENTRY = Number(process.env.WAYPOINT_ENTRY ?? 400100);

const DB_WORLD = process.env.DB_WORLD ?? "acore_world";
const DB_CHARACTERS = process.env.DB_CHARACTERS ?? "acore_characters";
const DB_PLAYERBOTS = process.env.DB_PLAYERBOTS ?? "acore_playerbots";

const DASHBOARD_CONTINENTS = (process.env.DASHBOARD_CONTINENTS ?? "0,1")
    .split(",")
    .map((s) => Number(s.trim()))
    .filter((n) => Number.isFinite(n));

// Race → faction mapping (matches Player::TeamIdForRace in C++ side).
const ALLIANCE_RACES = [1, 3, 4, 7, 11];
const HORDE_RACES = [2, 5, 6, 8, 10];

const pool = mysql.createPool({
    host: process.env.DB_HOST ?? "127.0.0.1",
    port: Number(process.env.DB_PORT ?? 3306),
    user: process.env.DB_USER ?? "root",
    password: process.env.DB_PASSWORD ?? "password",
    waitForConnections: true,
    connectionLimit: 8,
});

type Banner = {
    guid: number;
    entry: number;
    map: number;
    zoneId: number;
    areaId: number;
    x: number;
    y: number;
    z: number;
    faction: number;
    guildId: number;
    capturedAt: number | null;
};

type Waypoint = {
    guid: number;
    map: number;
    zoneId: number;
    areaId: number;
    x: number;
    y: number;
    z: number;
};

async function safe<T>(
    label: string,
    fn: () => Promise<T>,
    fallback: T,
): Promise<T> {
    try {
        return await fn();
    } catch (err) {
        console.warn(`[dashboard] ${label} unavailable:`, (err as Error).message);
        return fallback;
    }
}

async function fetchBanners(): Promise<Banner[]> {
    const sql = `
        SELECT g.guid, g.id, g.map, g.zoneId, g.areaId,
               g.position_x, g.position_y, g.position_z,
               COALESCE(c.faction, 0)      AS faction,
               COALESCE(c.guild_id, 0)     AS guild_id,
               c.captured_at               AS captured_at
        FROM \`${DB_WORLD}\`.\`gameobject\` g
        LEFT JOIN \`${DB_CHARACTERS}\`.\`conquest_zone_control\` c
               ON c.zone_guid = g.guid
        WHERE g.id = ?
        ORDER BY g.map, g.guid
    `;
    const [rows] = await pool.query<mysql.RowDataPacket[]>(sql, [BANNER_ENTRY]);
    return rows.map((r) => ({
        guid: Number(r.guid),
        entry: Number(r.id),
        map: Number(r.map),
        zoneId: Number(r.zoneId),
        areaId: Number(r.areaId),
        x: Number(r.position_x),
        y: Number(r.position_y),
        z: Number(r.position_z),
        faction: Number(r.faction),
        guildId: Number(r.guild_id),
        capturedAt: r.captured_at == null ? null : Number(r.captured_at),
    }));
}

async function fetchWaypoints(): Promise<Waypoint[]> {
    const sql = `
        SELECT guid, map, zoneId, areaId,
               position_x, position_y, position_z
        FROM \`${DB_WORLD}\`.\`gameobject\`
        WHERE id = ?
        ORDER BY map, guid
    `;
    const [rows] = await pool.query<mysql.RowDataPacket[]>(sql, [WAYPOINT_ENTRY]);
    return rows.map((r) => ({
        guid: Number(r.guid),
        map: Number(r.map),
        zoneId: Number(r.zoneId),
        areaId: Number(r.areaId),
        x: Number(r.position_x),
        y: Number(r.position_y),
        z: Number(r.position_z),
    }));
}

// ---------------------------------------------------------------- dashboard

type ZoneRow = {
    zoneId: number;
    zoneName: string;
    continentId: number;
    bannerCount: number;
    bannersAlliance: number;
    bannersHorde: number;
    bannersNeutral: number;
    guildIds: number[];
    capturedAtLatest: number | null;
    playersAlliance: number;
    playersHorde: number;
    bots: number;
};

async function fetchBannerRowsForZones(): Promise<
    Array<{
        guid: number;
        map: number;
        x: number;
        y: number;
        faction: number;
        guildId: number;
        capturedAt: number | null;
    }>
> {
    const sql = `
        SELECT g.guid, g.map, g.position_x, g.position_y,
               COALESCE(c.faction, 0)  AS faction,
               COALESCE(c.guild_id, 0) AS guild_id,
               c.captured_at           AS captured_at
        FROM \`${DB_WORLD}\`.\`gameobject\` g
        LEFT JOIN \`${DB_CHARACTERS}\`.\`conquest_zone_control\` c
               ON c.zone_guid = g.guid
        WHERE g.id = ?
    `;
    const [rows] = await pool.query<mysql.RowDataPacket[]>(sql, [BANNER_ENTRY]);
    return rows.map((r) => ({
        guid: Number(r.guid),
        map: Number(r.map),
        x: Number(r.position_x),
        y: Number(r.position_y),
        faction: Number(r.faction),
        guildId: Number(r.guild_id),
        capturedAt: r.captured_at == null ? null : Number(r.captured_at),
    }));
}

// Build per-zone aggregates by:
//  - Resolving each banner to its nearest known zone center on the same map
//    (because the user's DB has gameobject.zoneId=0 for all banners and the
//    areatable_dbc table is empty — see static-zones.ts)
//  - Returning one row per zone that has banners OR online players
async function fetchZoneBannerRows(continents: number[]): Promise<ZoneRow[]> {
    if (continents.length === 0) return [];
    const banners = await fetchBannerRowsForZones();

    const filtered = banners.filter((b) => continents.includes(b.map));
    const rowsByZone = new Map<number, ZoneRow>();

    const seedRow = (zoneId: number, mapId: number): ZoneRow => {
        return {
            zoneId,
            zoneName: zoneName(zoneId),
            continentId: mapId,
            bannerCount: 0,
            bannersAlliance: 0,
            bannersHorde: 0,
            bannersNeutral: 0,
            guildIds: [],
            capturedAtLatest: null,
            playersAlliance: 0,
            playersHorde: 0,
            bots: 0,
        };
    };

    for (const b of filtered) {
        const z = nearestZone(b.map, b.x, b.y);
        if (!z) continue;
        let row = rowsByZone.get(z.id);
        if (!row) {
            row = seedRow(z.id, z.mapId);
            rowsByZone.set(z.id, row);
        }
        row.bannerCount += 1;
        if (b.faction === 1) row.bannersAlliance += 1;
        else if (b.faction === 2) row.bannersHorde += 1;
        else row.bannersNeutral += 1;
        if (b.guildId > 0 && !row.guildIds.includes(b.guildId)) {
            row.guildIds.push(b.guildId);
        }
        if (
            b.capturedAt != null &&
            (row.capturedAtLatest == null || b.capturedAt > row.capturedAtLatest)
        ) {
            row.capturedAtLatest = b.capturedAt;
        }
    }

    return Array.from(rowsByZone.values()).sort((a, b) =>
        a.continentId !== b.continentId
            ? a.continentId - b.continentId
            : a.zoneName.localeCompare(b.zoneName),
    );
}

// Seed extra zone rows for zones with online players but no banner — so the
// dashboard reflects activity even outside contested zones.
function mergePlayerCountZones(
    rows: ZoneRow[],
    counts: PlayerCount[],
    continents: number[],
): ZoneRow[] {
    const byZone = new Map<number, ZoneRow>(rows.map((r) => [r.zoneId, r]));
    for (const pc of counts) {
        const z = STATIC_ZONES.find((s) => s.id === pc.zoneId);
        if (!z) continue;
        if (!continents.includes(z.mapId)) continue;
        let row = byZone.get(pc.zoneId);
        if (!row) {
            row = {
                zoneId: pc.zoneId,
                zoneName: z.name,
                continentId: z.mapId,
                bannerCount: 0,
                bannersAlliance: 0,
                bannersHorde: 0,
                bannersNeutral: 0,
                guildIds: [],
                capturedAtLatest: null,
                playersAlliance: 0,
                playersHorde: 0,
                bots: 0,
            };
            byZone.set(pc.zoneId, row);
        }
        row.playersAlliance = pc.playersAlliance;
        row.playersHorde = pc.playersHorde;
        row.bots = pc.bots;
    }
    return Array.from(byZone.values()).sort((a, b) =>
        a.continentId !== b.continentId
            ? a.continentId - b.continentId
            : a.zoneName.localeCompare(b.zoneName),
    );
}

type PlayerCount = {
    zoneId: number;
    playersAlliance: number;
    playersHorde: number;
    bots: number;
};

async function fetchPlayerCountsByZone(maps: number[]): Promise<PlayerCount[]> {
    if (maps.length === 0) return [];
    // Count ALL online characters by faction, including bots — on a
    // bot-heavy server the bot-aware Alliance/Horde counts are what reflect
    // actual zone activity, and bots are surfaced separately via the bots
    // column for transparency.
    const mapsPh = maps.map(() => "?").join(",");
    const ar = ALLIANCE_RACES.join(",");
    const hr = HORDE_RACES.join(",");
    const sql = `
        SELECT
            c.zone AS zone_id,
            SUM(CASE WHEN c.race IN (${ar}) THEN 1 ELSE 0 END) AS players_alliance,
            SUM(CASE WHEN c.race IN (${hr}) THEN 1 ELSE 0 END) AS players_horde,
            SUM(CASE WHEN COALESCE(pat.account_type,0) IN (1,2) THEN 1 ELSE 0 END) AS bots
        FROM \`${DB_CHARACTERS}\`.\`characters\` c
        LEFT JOIN \`${DB_PLAYERBOTS}\`.\`playerbots_account_type\` pat
               ON pat.account_id = c.account
        WHERE c.online = 1
          AND c.map IN (${mapsPh})
        GROUP BY c.zone
    `;
    const [rows] = await pool.query<mysql.RowDataPacket[]>(sql, maps);
    return rows.map((r) => ({
        zoneId: Number(r.zone_id),
        playersAlliance: Number(r.players_alliance ?? 0),
        playersHorde: Number(r.players_horde ?? 0),
        bots: Number(r.bots ?? 0),
    }));
}

// Same query but without the playerbots join, used as fallback when the
// playerbots DB isn't present.
async function fetchPlayerCountsByZoneNoBots(
    maps: number[],
): Promise<PlayerCount[]> {
    if (maps.length === 0) return [];
    const mapsPh = maps.map(() => "?").join(",");
    const ar = ALLIANCE_RACES.join(",");
    const hr = HORDE_RACES.join(",");
    const sql = `
        SELECT
            c.zone AS zone_id,
            SUM(CASE WHEN c.race IN (${ar}) THEN 1 ELSE 0 END) AS players_alliance,
            SUM(CASE WHEN c.race IN (${hr}) THEN 1 ELSE 0 END) AS players_horde,
            0 AS bots
        FROM \`${DB_CHARACTERS}\`.\`characters\` c
        WHERE c.online = 1
          AND c.map IN (${mapsPh})
        GROUP BY c.zone
    `;
    const [rows] = await pool.query<mysql.RowDataPacket[]>(sql, maps);
    return rows.map((r) => ({
        zoneId: Number(r.zone_id),
        playersAlliance: Number(r.players_alliance ?? 0),
        playersHorde: Number(r.players_horde ?? 0),
        bots: 0,
    }));
}

type BotCounts = {
    totalOnline: number;
    rndBot: number;
    addClass: number;
    byMap: { mapId: number; count: number }[];
};

async function fetchBotCounts(maps: number[]): Promise<BotCounts | null> {
    const sql = `
        SELECT
            c.map AS map_id,
            pat.account_type AS bot_type,
            COUNT(*) AS n
        FROM \`${DB_CHARACTERS}\`.\`characters\` c
        INNER JOIN \`${DB_PLAYERBOTS}\`.\`playerbots_account_type\` pat
               ON pat.account_id = c.account
        WHERE c.online = 1
          AND pat.account_type IN (1, 2)
        GROUP BY c.map, pat.account_type
    `;
    const [rows] = await pool.query<mysql.RowDataPacket[]>(sql);
    const byMap = new Map<number, number>();
    let rnd = 0;
    let addc = 0;
    for (const r of rows) {
        const m = Number(r.map_id);
        const t = Number(r.bot_type);
        const n = Number(r.n);
        byMap.set(m, (byMap.get(m) ?? 0) + n);
        if (t === 1) rnd += n;
        else if (t === 2) addc += n;
    }
    return {
        totalOnline: rnd + addc,
        rndBot: rnd,
        addClass: addc,
        byMap: maps.map((m) => ({ mapId: m, count: byMap.get(m) ?? 0 })),
    };
}

type TopPlayer = {
    guid: number;
    name: string;
    race: number;
    faction: number;
    online: boolean;
    kills: number;
    deaths: number;
    kd: number | null;
    currentStreak: number;
    maxStreak: number;
};

async function fetchTopPlayers(limit = 50): Promise<TopPlayer[]> {
    // conquest_player_stats lives in acore_world (mod-conquest-core, currently
    // disabled — table may be missing). character_conquest_killstreak lives in
    // acore_characters (mod-conquest-frontline, active).
    const sql = `
        SELECT
            c.guid, c.name, c.race, c.online,
            COALESCE(cps.kills, 0)         AS kills,
            COALESCE(cps.deaths, 0)        AS deaths,
            COALESCE(cks.current_streak,0) AS current_streak,
            COALESCE(cks.max_streak,0)     AS max_streak
        FROM \`${DB_CHARACTERS}\`.\`characters\` c
        LEFT JOIN \`${DB_WORLD}\`.\`conquest_player_stats\` cps
               ON cps.player_guid = c.guid
        LEFT JOIN \`${DB_CHARACTERS}\`.\`character_conquest_killstreak\` cks
               ON cks.guid = c.guid
        WHERE COALESCE(cps.kills,0) > 0
           OR COALESCE(cps.deaths,0) > 0
           OR COALESCE(cks.max_streak,0) > 0
           OR COALESCE(cks.current_streak,0) > 0
        ORDER BY kills DESC, max_streak DESC
        LIMIT ?
    `;
    const [rows] = await pool.query<mysql.RowDataPacket[]>(sql, [limit]);
    return rows.map((r) => {
        const race = Number(r.race);
        const faction = ALLIANCE_RACES.includes(race)
            ? 1
            : HORDE_RACES.includes(race)
              ? 2
              : 0;
        const kills = Number(r.kills);
        const deaths = Number(r.deaths);
        return {
            guid: Number(r.guid),
            name: String(r.name),
            race,
            faction,
            online: Number(r.online) === 1,
            kills,
            deaths,
            kd: deaths > 0 ? kills / deaths : kills > 0 ? null : 0,
            currentStreak: Number(r.current_streak),
            maxStreak: Number(r.max_streak),
        };
    });
}

// Fallback when conquest_player_stats is missing — query only killstreak.
async function fetchTopKillstreaks(limit = 50): Promise<TopPlayer[]> {
    const sql = `
        SELECT
            c.guid, c.name, c.race, c.online,
            COALESCE(cks.current_streak,0) AS current_streak,
            COALESCE(cks.max_streak,0)     AS max_streak
        FROM \`${DB_CHARACTERS}\`.\`characters\` c
        INNER JOIN \`${DB_CHARACTERS}\`.\`character_conquest_killstreak\` cks
                ON cks.guid = c.guid
        WHERE cks.max_streak > 0 OR cks.current_streak > 0
        ORDER BY max_streak DESC, current_streak DESC
        LIMIT ?
    `;
    const [rows] = await pool.query<mysql.RowDataPacket[]>(sql, [limit]);
    return rows.map((r) => {
        const race = Number(r.race);
        const faction = ALLIANCE_RACES.includes(race)
            ? 1
            : HORDE_RACES.includes(race)
              ? 2
              : 0;
        return {
            guid: Number(r.guid),
            name: String(r.name),
            race,
            faction,
            online: Number(r.online) === 1,
            kills: 0,
            deaths: 0,
            kd: 0,
            currentStreak: Number(r.current_streak),
            maxStreak: Number(r.max_streak),
        };
    });
}

type GuildInfo = { id: number; name: string };

async function fetchGuilds(ids: number[]): Promise<GuildInfo[]> {
    if (ids.length === 0) return [];
    const placeholders = ids.map(() => "?").join(",");
    const sql = `
        SELECT guildid AS id, name
        FROM \`${DB_CHARACTERS}\`.\`guild\`
        WHERE guildid IN (${placeholders})
    `;
    const [rows] = await pool.query<mysql.RowDataPacket[]>(sql, ids);
    return rows.map((r) => ({ id: Number(r.id), name: String(r.name) }));
}

async function buildDashboard() {
    const bannerZones = await safe(
        "banner→zone aggregates",
        () => fetchZoneBannerRows(DASHBOARD_CONTINENTS),
        [],
    );

    const continentMaps = Array.from(
        new Set(DASHBOARD_CONTINENTS),
    ).filter((m) => Number.isFinite(m));

    let playerCounts = await safe(
        "player counts (with bots)",
        () => fetchPlayerCountsByZone(continentMaps),
        null as PlayerCount[] | null,
    );
    let botDbAvailable = playerCounts !== null;
    if (!botDbAvailable) {
        playerCounts = await safe(
            "player counts (no bots)",
            () => fetchPlayerCountsByZoneNoBots(continentMaps),
            [],
        );
    }

    const zones = mergePlayerCountZones(
        bannerZones,
        playerCounts ?? [],
        DASHBOARD_CONTINENTS,
    );

    const bots = botDbAvailable
        ? await safe("bot summary", () => fetchBotCounts(continentMaps), null)
        : null;

    let topPlayers = await safe(
        "top players (kills + streak)",
        () => fetchTopPlayers(50),
        null as TopPlayer[] | null,
    );
    if (!topPlayers) {
        topPlayers = await safe(
            "top killstreaks (fallback)",
            () => fetchTopKillstreaks(50),
            [],
        );
    }

    const guildIds = Array.from(
        new Set(zones.flatMap((z) => z.guildIds)),
    ).filter((n) => n > 0);
    const guilds = await safe(
        "guild names",
        () => fetchGuilds(guildIds),
        [] as GuildInfo[],
    );

    // KPIs
    const totalBanners = zones.reduce((s, z) => s + z.bannerCount, 0);
    const allianceBanners = zones.reduce((s, z) => s + z.bannersAlliance, 0);
    const hordeBanners = zones.reduce((s, z) => s + z.bannersHorde, 0);
    const neutralBanners = zones.reduce((s, z) => s + z.bannersNeutral, 0);
    const playersA = zones.reduce((s, z) => s + z.playersAlliance, 0);
    const playersH = zones.reduce((s, z) => s + z.playersHorde, 0);
    const botsTotal = zones.reduce((s, z) => s + z.bots, 0);

    // Most-controlled zone (current state). NOTE: no historical capture log
    // exists, so we expose "most banners currently owned by one faction" as a
    // proxy for the user's "zone la plus capturée" question.
    const mostControlled = [...zones]
        .map((z) => ({
            zone: z,
            dominant: Math.max(z.bannersAlliance, z.bannersHorde),
        }))
        .filter((x) => x.dominant > 0)
        .sort((a, b) => b.dominant - a.dominant)[0]?.zone ?? null;

    const mostRecentCapture = [...zones]
        .filter((z) => z.capturedAtLatest != null)
        .sort(
            (a, b) =>
                (b.capturedAtLatest ?? 0) - (a.capturedAtLatest ?? 0),
        )[0] ?? null;

    return {
        zones,
        guilds,
        bots,
        topPlayers,
        kpis: {
            totalBanners,
            allianceBanners,
            hordeBanners,
            neutralBanners,
            playersAlliance: playersA,
            playersHorde: playersH,
            totalPlayers: playersA + playersH,
            totalBots: botsTotal,
            mostControlledZone: mostControlled
                ? {
                      zoneId: mostControlled.zoneId,
                      zoneName: mostControlled.zoneName,
                      dominantFaction:
                          mostControlled.bannersAlliance >=
                          mostControlled.bannersHorde
                              ? 1
                              : 2,
                      bannersAlliance: mostControlled.bannersAlliance,
                      bannersHorde: mostControlled.bannersHorde,
                  }
                : null,
            mostRecentCapture: mostRecentCapture
                ? {
                      zoneId: mostRecentCapture.zoneId,
                      zoneName: mostRecentCapture.zoneName,
                      capturedAt: mostRecentCapture.capturedAtLatest,
                  }
                : null,
        },
        meta: {
            continents: DASHBOARD_CONTINENTS,
            botDbAvailable,
            generatedAt: Math.floor(Date.now() / 1000),
        },
    };
}

const app = express();
app.use(cors());

app.get("/api/health", async (_req, res) => {
    try {
        await pool.query("SELECT 1");
        res.json({ ok: true });
    } catch (err) {
        res.status(500).json({ ok: false, error: String(err) });
    }
});

app.get("/api/banners", async (_req, res) => {
    try {
        res.json(await fetchBanners());
    } catch (err) {
        console.error("[banners]", err);
        res.status(500).json({ error: String(err) });
    }
});

app.get("/api/waypoints", async (_req, res) => {
    try {
        res.json(await fetchWaypoints());
    } catch (err) {
        console.error("[waypoints]", err);
        res.status(500).json({ error: String(err) });
    }
});

app.get("/api/all", async (_req, res) => {
    try {
        const [banners, waypoints] = await Promise.all([
            fetchBanners(),
            fetchWaypoints(),
        ]);
        res.json({ banners, waypoints });
    } catch (err) {
        console.error("[all]", err);
        res.status(500).json({ error: String(err) });
    }
});

app.get("/api/dashboard", async (_req, res) => {
    try {
        res.json(await buildDashboard());
    } catch (err) {
        console.error("[dashboard]", err);
        res.status(500).json({ error: String(err) });
    }
});


app.listen(PORT, () => {
    console.log(`conquest-map server listening on :${PORT}`);
    console.log(`banner entry=${BANNER_ENTRY}  waypoint entry=${WAYPOINT_ENTRY}`);
    console.log(`dashboard continents=${DASHBOARD_CONTINENTS.join(",")}`);
});
