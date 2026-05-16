import { useEffect, useMemo, useState } from "react";
import { fetchDashboard } from "./api";
import type {
    DashboardData,
    TopPlayer,
    ZoneRow,
} from "./dashboard-types";
import { FACTION_COLOR, FACTION_LABEL } from "./types";

const REFRESH_MS = 10_000;

const CONTINENT_NAME: Record<number, string> = {
    0: "Eastern Kingdoms",
    1: "Kalimdor",
};

function formatRelative(unix: number | null): string {
    if (unix == null || unix === 0) return "—";
    const diffSec = Math.floor(Date.now() / 1000) - unix;
    if (diffSec < 60) return `il y a ${diffSec}s`;
    if (diffSec < 3600) return `il y a ${Math.floor(diffSec / 60)} min`;
    if (diffSec < 86400) return `il y a ${Math.floor(diffSec / 3600)} h`;
    return `il y a ${Math.floor(diffSec / 86400)} j`;
}

type SortDir = "asc" | "desc";

export function Dashboard({ autoRefresh }: { autoRefresh: boolean }) {
    const [data, setData] = useState<DashboardData | null>(null);
    const [err, setErr] = useState<string | null>(null);
    const [continentFilter, setContinentFilter] = useState<Set<number>>(
        new Set([0, 1]),
    );
    const [zoneSort, setZoneSort] = useState<{
        key: keyof ZoneRow;
        dir: SortDir;
    }>({ key: "playersAlliance", dir: "desc" });
    const [playerSort, setPlayerSort] = useState<{
        key: keyof TopPlayer;
        dir: SortDir;
    }>({ key: "kills", dir: "desc" });

    useEffect(() => {
        let mounted = true;
        const load = () =>
            fetchDashboard()
                .then((d) => {
                    if (mounted) {
                        setData(d);
                        setErr(null);
                    }
                })
                .catch((e: unknown) => {
                    if (mounted)
                        setErr(
                            e instanceof Error ? e.message : String(e),
                        );
                });
        void load();
        if (!autoRefresh) return;
        const id = window.setInterval(load, REFRESH_MS);
        return () => {
            mounted = false;
            window.clearInterval(id);
        };
    }, [autoRefresh]);

    const guildNameById = useMemo(() => {
        const m = new Map<number, string>();
        for (const g of data?.guilds ?? []) m.set(g.id, g.name);
        return m;
    }, [data]);

    const visibleZones = useMemo(() => {
        if (!data) return [];
        const filtered = data.zones.filter((z) =>
            continentFilter.has(z.continentId),
        );
        const { key, dir } = zoneSort;
        return [...filtered].sort((a, b) => {
            const va = a[key];
            const vb = b[key];
            if (typeof va === "number" && typeof vb === "number") {
                return dir === "asc" ? va - vb : vb - va;
            }
            const sa = String(va ?? "");
            const sb = String(vb ?? "");
            return dir === "asc" ? sa.localeCompare(sb) : sb.localeCompare(sa);
        });
    }, [data, continentFilter, zoneSort]);

    const visiblePlayers = useMemo(() => {
        if (!data) return [];
        const { key, dir } = playerSort;
        return [...data.topPlayers].sort((a, b) => {
            const va = a[key];
            const vb = b[key];
            if (typeof va === "number" && typeof vb === "number") {
                return dir === "asc" ? va - vb : vb - va;
            }
            const sa = String(va ?? "");
            const sb = String(vb ?? "");
            return dir === "asc" ? sa.localeCompare(sb) : sb.localeCompare(sa);
        });
    }, [data, playerSort]);

    if (err)
        return (
            <div className="dashboard error">Erreur dashboard: {err}</div>
        );
    if (!data) return <div className="dashboard loading">chargement…</div>;

    const k = data.kpis;

    return (
        <div className="dashboard">
            <div className="dashboard-toolbar">
                <span className="ds-label">Continents :</span>
                {[0, 1].map((cid) => (
                    <label key={cid} className="toggle">
                        <input
                            type="checkbox"
                            checked={continentFilter.has(cid)}
                            onChange={(e) => {
                                setContinentFilter((prev) => {
                                    const next = new Set(prev);
                                    if (e.target.checked) next.add(cid);
                                    else next.delete(cid);
                                    return next;
                                });
                            }}
                        />
                        {CONTINENT_NAME[cid]}
                    </label>
                ))}
                {!data.meta.botDbAvailable && (
                    <span className="warn">
                        ⚠ DB playerbots indisponible — comptage bots = 0
                    </span>
                )}
            </div>

            <section className="kpis">
                <Kpi label="Bannières 400010" value={k.totalBanners} />
                <Kpi
                    label="Contrôlées Alliance"
                    value={k.allianceBanners}
                    color={FACTION_COLOR[1]}
                />
                <Kpi
                    label="Contrôlées Horde"
                    value={k.hordeBanners}
                    color={FACTION_COLOR[2]}
                />
                <Kpi
                    label="Neutres / disputées"
                    value={k.neutralBanners}
                    color={FACTION_COLOR[0]}
                />
                <Kpi
                    label="Joueurs online (EK+Kal)"
                    value={k.totalPlayers}
                    sub={`A:${k.playersAlliance} · H:${k.playersHorde}`}
                />
                <Kpi
                    label="Bots online (EK+Kal)"
                    value={k.totalBots}
                    sub={
                        data.bots
                            ? `RND:${data.bots.rndBot} · AddClass:${data.bots.addClass}`
                            : undefined
                    }
                />
                <Kpi
                    label="Zone la + contrôlée"
                    value={
                        k.mostControlledZone?.zoneName ?? "—"
                    }
                    sub={
                        k.mostControlledZone
                            ? `A:${k.mostControlledZone.bannersAlliance} · H:${k.mostControlledZone.bannersHorde}`
                            : "aucune capture"
                    }
                />
                <Kpi
                    label="Dernière capture"
                    value={
                        k.mostRecentCapture?.zoneName ?? "—"
                    }
                    sub={formatRelative(
                        k.mostRecentCapture?.capturedAt ?? null,
                    )}
                />
            </section>

            <section className="card">
                <header className="card-head">
                    <h2>Zones</h2>
                    <span className="card-hint">
                        cliquer un en-tête pour trier · {visibleZones.length}{" "}
                        zones avec bannières
                    </span>
                </header>
                <div className="dash-table-wrap"><table className="dash-table">
                    <thead>
                        <tr>
                            <ZoneTh
                                k="zoneName"
                                sort={zoneSort}
                                set={setZoneSort}
                            >
                                Zone
                            </ZoneTh>
                            <ZoneTh
                                k="continentId"
                                sort={zoneSort}
                                set={setZoneSort}
                            >
                                Continent
                            </ZoneTh>
                            <ZoneTh
                                k="bannerCount"
                                sort={zoneSort}
                                set={setZoneSort}
                            >
                                Bannières
                            </ZoneTh>
                            <ZoneTh
                                k="bannersAlliance"
                                sort={zoneSort}
                                set={setZoneSort}
                            >
                                A
                            </ZoneTh>
                            <ZoneTh
                                k="bannersHorde"
                                sort={zoneSort}
                                set={setZoneSort}
                            >
                                H
                            </ZoneTh>
                            <ZoneTh
                                k="bannersNeutral"
                                sort={zoneSort}
                                set={setZoneSort}
                            >
                                Neutre
                            </ZoneTh>
                            <th>Guildes</th>
                            <ZoneTh
                                k="playersAlliance"
                                sort={zoneSort}
                                set={setZoneSort}
                            >
                                Joueurs A
                            </ZoneTh>
                            <ZoneTh
                                k="playersHorde"
                                sort={zoneSort}
                                set={setZoneSort}
                            >
                                Joueurs H
                            </ZoneTh>
                            <ZoneTh k="bots" sort={zoneSort} set={setZoneSort}>
                                Bots
                            </ZoneTh>
                            <ZoneTh
                                k="capturedAtLatest"
                                sort={zoneSort}
                                set={setZoneSort}
                            >
                                Dernière cap.
                            </ZoneTh>
                        </tr>
                    </thead>
                    <tbody>
                        {visibleZones.map((z) => (
                            <tr key={z.zoneId}>
                                <td className="strong">{z.zoneName}</td>
                                <td>{CONTINENT_NAME[z.continentId] ?? z.continentId}</td>
                                <td className="num">{z.bannerCount}</td>
                                <td className="num faction-a">
                                    {z.bannersAlliance}
                                </td>
                                <td className="num faction-h">
                                    {z.bannersHorde}
                                </td>
                                <td className="num muted">
                                    {z.bannersNeutral}
                                </td>
                                <td className="guilds">
                                    {z.guildIds.length === 0 ? (
                                        <span className="muted">—</span>
                                    ) : (
                                        z.guildIds
                                            .map(
                                                (id) =>
                                                    guildNameById.get(id) ??
                                                    `#${id}`,
                                            )
                                            .join(", ")
                                    )}
                                </td>
                                <td className="num faction-a">
                                    {z.playersAlliance}
                                </td>
                                <td className="num faction-h">
                                    {z.playersHorde}
                                </td>
                                <td className="num">{z.bots}</td>
                                <td className="muted">
                                    {formatRelative(z.capturedAtLatest)}
                                </td>
                            </tr>
                        ))}
                        {visibleZones.length === 0 && (
                            <tr>
                                <td colSpan={11} className="empty-row">
                                    Aucune zone avec bannière sur ce filtre.
                                </td>
                            </tr>
                        )}
                    </tbody>
                </table></div>
            </section>

            <section className="card">
                <header className="card-head">
                    <h2>Bots online par map</h2>
                </header>
                {data.bots ? (
                    <div className="dash-table-wrap">
                        <table className="dash-table compact">
                            <thead>
                                <tr>
                                    <th>Map</th>
                                    <th className="num">Bots</th>
                                </tr>
                            </thead>
                            <tbody>
                                {data.bots.byMap.map((row) => (
                                    <tr key={row.mapId}>
                                        <td>
                                            {CONTINENT_NAME[row.mapId] ??
                                                `map ${row.mapId}`}
                                        </td>
                                        <td className="num">{row.count}</td>
                                    </tr>
                                ))}
                                <tr className="total-row">
                                    <td>Total</td>
                                    <td className="num">
                                        {data.bots.totalOnline}
                                    </td>
                                </tr>
                            </tbody>
                        </table>
                    </div>
                ) : (
                    <div className="muted card-body-padding">
                        DB playerbots indisponible.
                    </div>
                )}
            </section>

            <section className="card">
                <header className="card-head">
                    <h2>Top joueurs (kills + killstreaks)</h2>
                    <span className="card-hint">
                        {visiblePlayers.length} entrées · cliquer un en-tête
                        pour trier
                    </span>
                </header>
                <div className="dash-table-wrap"><table className="dash-table">
                    <thead>
                        <tr>
                            <PlayerTh
                                k="name"
                                sort={playerSort}
                                set={setPlayerSort}
                            >
                                Nom
                            </PlayerTh>
                            <PlayerTh
                                k="faction"
                                sort={playerSort}
                                set={setPlayerSort}
                            >
                                Faction
                            </PlayerTh>
                            <PlayerTh
                                k="online"
                                sort={playerSort}
                                set={setPlayerSort}
                            >
                                Online
                            </PlayerTh>
                            <PlayerTh
                                k="kills"
                                sort={playerSort}
                                set={setPlayerSort}
                            >
                                Kills
                            </PlayerTh>
                            <PlayerTh
                                k="deaths"
                                sort={playerSort}
                                set={setPlayerSort}
                            >
                                Deaths
                            </PlayerTh>
                            <PlayerTh
                                k="kd"
                                sort={playerSort}
                                set={setPlayerSort}
                            >
                                K/D
                            </PlayerTh>
                            <PlayerTh
                                k="currentStreak"
                                sort={playerSort}
                                set={setPlayerSort}
                            >
                                Streak
                            </PlayerTh>
                            <PlayerTh
                                k="maxStreak"
                                sort={playerSort}
                                set={setPlayerSort}
                            >
                                Max streak
                            </PlayerTh>
                        </tr>
                    </thead>
                    <tbody>
                        {visiblePlayers.map((p) => (
                            <tr key={p.guid}>
                                <td className="strong">{p.name}</td>
                                <td>
                                    <span
                                        className="dot"
                                        style={{
                                            background: FACTION_COLOR[p.faction],
                                        }}
                                    />
                                    {FACTION_LABEL[p.faction] ?? "?"}
                                </td>
                                <td>{p.online ? "✓" : "—"}</td>
                                <td className="num">{p.kills}</td>
                                <td className="num">{p.deaths}</td>
                                <td className="num">
                                    {p.kd == null
                                        ? "∞"
                                        : p.kd.toFixed(2)}
                                </td>
                                <td className="num">{p.currentStreak}</td>
                                <td className="num">{p.maxStreak}</td>
                            </tr>
                        ))}
                        {visiblePlayers.length === 0 && (
                            <tr>
                                <td colSpan={8} className="empty-row">
                                    Aucune stat de joueur enregistrée.
                                </td>
                            </tr>
                        )}
                    </tbody>
                </table></div>
                <footer className="card-foot muted">
                    Note : kills / deaths viennent de
                    <code> conquest_player_stats </code>
                    (module
                    <code> mod-conquest-core </code>désactivé). Les killstreaks
                    sont alimentés par
                    <code> mod-conquest-frontline </code>actif.
                </footer>
            </section>
        </div>
    );
}

function Kpi({
    label,
    value,
    sub,
    color,
}: {
    label: string;
    value: number | string;
    sub?: string;
    color?: string;
}) {
    return (
        <div className="kpi">
            <div className="kpi-label">{label}</div>
            <div
                className="kpi-value"
                style={color ? { color } : undefined}
                title={String(value)}
            >
                {value}
            </div>
            {sub && <div className="kpi-sub">{sub}</div>}
        </div>
    );
}

function ZoneTh({
    k,
    sort,
    set,
    children,
}: {
    k: keyof ZoneRow;
    sort: { key: keyof ZoneRow; dir: SortDir };
    set: (s: { key: keyof ZoneRow; dir: SortDir }) => void;
    children: React.ReactNode;
}) {
    return (
        <th
            className="sortable"
            onClick={() =>
                set({
                    key: k,
                    dir: sort.key === k && sort.dir === "desc" ? "asc" : "desc",
                })
            }
        >
            {children}
            {sort.key === k && (
                <span className="sort-arrow">
                    {sort.dir === "desc" ? "▼" : "▲"}
                </span>
            )}
        </th>
    );
}

function PlayerTh({
    k,
    sort,
    set,
    children,
}: {
    k: keyof TopPlayer;
    sort: { key: keyof TopPlayer; dir: SortDir };
    set: (s: { key: keyof TopPlayer; dir: SortDir }) => void;
    children: React.ReactNode;
}) {
    return (
        <th
            className="sortable"
            onClick={() =>
                set({
                    key: k,
                    dir: sort.key === k && sort.dir === "desc" ? "asc" : "desc",
                })
            }
        >
            {children}
            {sort.key === k && (
                <span className="sort-arrow">
                    {sort.dir === "desc" ? "▼" : "▲"}
                </span>
            )}
        </th>
    );
}
