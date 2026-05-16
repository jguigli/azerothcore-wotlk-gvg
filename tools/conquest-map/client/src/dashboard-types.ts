export type ZoneRow = {
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

export type GuildInfo = { id: number; name: string };

export type BotCounts = {
    totalOnline: number;
    rndBot: number;
    addClass: number;
    byMap: { mapId: number; count: number }[];
};

export type TopPlayer = {
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

export type DashboardData = {
    zones: ZoneRow[];
    guilds: GuildInfo[];
    bots: BotCounts | null;
    topPlayers: TopPlayer[];
    kpis: {
        totalBanners: number;
        allianceBanners: number;
        hordeBanners: number;
        neutralBanners: number;
        playersAlliance: number;
        playersHorde: number;
        totalPlayers: number;
        totalBots: number;
        mostControlledZone:
            | {
                  zoneId: number;
                  zoneName: string;
                  dominantFaction: number;
                  bannersAlliance: number;
                  bannersHorde: number;
              }
            | null;
        mostRecentCapture:
            | {
                  zoneId: number;
                  zoneName: string;
                  capturedAt: number | null;
              }
            | null;
    };
    meta: {
        continents: number[];
        botDbAvailable: boolean;
        generatedAt: number;
    };
};
