export type Banner = {
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

export type Waypoint = {
    guid: number;
    map: number;
    zoneId: number;
    areaId: number;
    x: number;
    y: number;
    z: number;
};

export type Faction = 0 | 1 | 2;

export const FACTION_COLOR: Record<number, string> = {
    0: "#9aa0b2", // neutral/disputed
    1: "#4aa3ff", // alliance
    2: "#ff4a5b", // horde
};

export const FACTION_LABEL: Record<number, string> = {
    0: "Neutre",
    1: "Alliance",
    2: "Horde",
};
