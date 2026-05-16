// Static EK + Kalimdor zone catalog.
//
// The user's DB has empty `areatable_dbc` and `worldmaparea_dbc` tables, so we
// can't resolve zone names or coordinate→zone containment from MySQL. This
// catalog bridges the gap: zone ID → (mapId, frFR name, approximate center
// X/Y in world coords). Centers come from known banner spawn positions in
// modules/mod-conquest-frontline/data/sql/db-world/conquest_banners_*.sql and
// from public Blizzard WorldMap data for zones without banners.
//
// Used for two things:
//   1. Resolving a banner's zone by picking the nearest center on the same map
//   2. Displaying a friendly zone name in the dashboard
//
// Inaccurate centers won't break the dashboard — they just affect which zone
// a borderline banner gets assigned to. Tune as needed.

export type StaticZone = {
    id: number;
    mapId: number; // 0 = EK, 1 = Kalimdor
    name: string;
    centerX: number;
    centerY: number;
};

export const STATIC_ZONES: StaticZone[] = [
    // ---- Eastern Kingdoms (map 0) ----
    { id: 1, mapId: 0, name: "Dun Morogh", centerX: -5300, centerY: -800 },
    { id: 3, mapId: 0, name: "Les Terres ingrates", centerX: -6700, centerY: -3300 },
    { id: 4, mapId: 0, name: "Terres foudroyées", centerX: -11000, centerY: -3300 },
    { id: 8, mapId: 0, name: "Marécage des Chagrins", centerX: -10400, centerY: -3300 },
    { id: 10, mapId: 0, name: "Bois de la Pénombre", centerX: -10520, centerY: -1170 },
    { id: 11, mapId: 0, name: "Les Paluns", centerX: -3812, centerY: -775 },
    { id: 12, mapId: 0, name: "Forêt d'Elwynn", centerX: -9446, centerY: 73 },
    { id: 28, mapId: 0, name: "Maleterres de l'Ouest", centerX: 1300, centerY: -1100 },
    { id: 33, mapId: 0, name: "Strangleronce", centerX: -12423, centerY: 156 },
    { id: 36, mapId: 0, name: "Montagnes d'Alterac", centerX: 200, centerY: -800 },
    { id: 38, mapId: 0, name: "Loch Modan", centerX: -5429, centerY: -2926 },
    { id: 40, mapId: 0, name: "Marche de l'Ouest", centerX: -10630, centerY: 1037 },
    { id: 41, mapId: 0, name: "Col de Sombrevent", centerX: -10400, centerY: -1500 },
    { id: 44, mapId: 0, name: "Les Carmines", centerX: -9268, centerY: -2299 },
    { id: 45, mapId: 0, name: "Hautes-terres d'Arathi", centerX: -1265, centerY: -2602 },
    { id: 46, mapId: 0, name: "Steppes ardentes", centerX: -7900, centerY: -2200 },
    { id: 47, mapId: 0, name: "Les Hinterlands", centerX: 287, centerY: -2024 },
    { id: 51, mapId: 0, name: "Gorge des vents brûlants", centerX: -6700, centerY: -1300 },
    { id: 85, mapId: 0, name: "Clairières de Tirisfal", centerX: 2271, centerY: 244 },
    { id: 130, mapId: 0, name: "Forêt des Pins argentés", centerX: -579, centerY: 1495 },
    { id: 139, mapId: 0, name: "Maleterres de l'Est", centerX: 1900, centerY: -3850 },
    { id: 267, mapId: 0, name: "Contreforts de Hautebrande", centerX: -200, centerY: 100 },
    { id: 1497, mapId: 0, name: "Fossoyeuse", centerX: 1700, centerY: 240 },
    { id: 1519, mapId: 0, name: "Hurlevent", centerX: -8900, centerY: 600 },
    { id: 1537, mapId: 0, name: "Forgefer", centerX: -4900, centerY: -940 },
    { id: 3430, mapId: 0, name: "Bois des Chants éternels", centerX: 8800, centerY: -7100 },
    { id: 3433, mapId: 0, name: "Bois des Esprits", centerX: 7800, centerY: -6700 },
    { id: 3487, mapId: 0, name: "Lune-d'argent", centerX: 9450, centerY: -7500 },
    { id: 4080, mapId: 0, name: "Île de Quel'Danas", centerX: 12947, centerY: -6893 },

    // ---- Kalimdor (map 1) ----
    { id: 14, mapId: 1, name: "Durotar", centerX: 379, centerY: -4707 },
    { id: 15, mapId: 1, name: "Marais des Chagrins (Dustwallow)", centerX: -3600, centerY: -3000 },
    { id: 16, mapId: 1, name: "Azshara", centerX: 3000, centerY: -4500 },
    { id: 17, mapId: 1, name: "Les Tarides (Barrens)", centerX: -435, centerY: -2660 },
    { id: 141, mapId: 1, name: "Teldrassil", centerX: 10300, centerY: 2400 },
    { id: 148, mapId: 1, name: "Sombrivage", centerX: 6404, centerY: 514 },
    { id: 215, mapId: 1, name: "Mulgore", centerX: -2371, centerY: -355 },
    { id: 331, mapId: 1, name: "Orneval (Ashenvale)", centerX: 2400, centerY: -900 },
    { id: 357, mapId: 1, name: "Féralas", centerX: -4400, centerY: 1500 },
    { id: 361, mapId: 1, name: "Bois de la Pénombre (Felwood)", centerX: 5400, centerY: -800 },
    { id: 400, mapId: 1, name: "Mille pointes", centerX: -4200, centerY: -1900 },
    { id: 405, mapId: 1, name: "Désolace", centerX: -3724, centerY: -3500 },
    { id: 406, mapId: 1, name: "Serres-Rocheuses", centerX: 1900, centerY: 300 },
    { id: 440, mapId: 1, name: "Tanaris", centerX: -7100, centerY: -3700 },
    { id: 490, mapId: 1, name: "Cratère d'Un'Goro", centerX: -6700, centerY: -1300 },
    { id: 493, mapId: 1, name: "Reflet-de-Lune", centerX: 7700, centerY: -2400 },
    { id: 618, mapId: 1, name: "Berceau-de-l'Hiver", centerX: 6900, centerY: -4400 },
    { id: 1377, mapId: 1, name: "Silithus", centerX: -6800, centerY: 800 },
    { id: 1637, mapId: 1, name: "Orgrimmar", centerX: 1500, centerY: -4400 },
    { id: 1638, mapId: 1, name: "Les Pitons-du-Tonnerre", centerX: -1300, centerY: 100 },
    { id: 1657, mapId: 1, name: "Darnassus", centerX: 9900, centerY: 2200 },
    { id: 3524, mapId: 1, name: "Île d'Azuremyst", centerX: -4015, centerY: -11724 },
    { id: 3525, mapId: 1, name: "Île de Sangrenuit", centerX: -2090, centerY: -11900 },
    { id: 3557, mapId: 1, name: "L'Exodar", centerX: -3987, centerY: -11789 },
];

const byId = new Map<number, StaticZone>(STATIC_ZONES.map((z) => [z.id, z]));

export function zoneById(id: number): StaticZone | undefined {
    return byId.get(id);
}

export function zoneName(id: number): string {
    return byId.get(id)?.name ?? `zone ${id}`;
}

/** Returns the nearest zone center on the given map. null if no zone exists
 *  for that map in the catalog. */
export function nearestZone(
    mapId: number,
    x: number,
    y: number,
): StaticZone | null {
    let best: StaticZone | null = null;
    let bestD = Infinity;
    for (const z of STATIC_ZONES) {
        if (z.mapId !== mapId) continue;
        const dx = z.centerX - x;
        const dy = z.centerY - y;
        const d = dx * dx + dy * dy;
        if (d < bestD) {
            bestD = d;
            best = z;
        }
    }
    return best;
}
