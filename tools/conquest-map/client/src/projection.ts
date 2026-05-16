// Projection from WoW world coordinates (x, y) to image pixel coordinates.
//
// WoW axes: +X = north, +Y = west. A continent map image is rendered north-up,
// so west (high Y) is on the LEFT and north (high X) is on the TOP.
//
//   pixelX = (yMax - worldY) / (yMax - yMin) * imageWidth
//   pixelY = (xMax - worldX) / (xMax - xMin) * imageHeight
//
// Bounds are approximate from Blizzard WorldMapArea data; the user may tune
// them per map image. To recalibrate: pick a known landmark, compare its
// expected pixel position with the projected one, and adjust xMin/xMax/yMin/yMax.

export type MapBounds = {
    xMin: number;
    xMax: number;
    yMin: number;
    yMax: number;
};

export type ContinentDef = {
    mapId: number;
    name: string;
    image: string; // path under /maps
    bounds: MapBounds;
};

export const CONTINENTS: ContinentDef[] = [
    {
        mapId: 0,
        name: "Eastern Kingdoms",
        image: "/maps/ek.webp",
        bounds: { xMin: -7916, xMax: 4609, yMin: -11464, yMax: 2069 },
    },
    {
        mapId: 1,
        name: "Kalimdor",
        image: "/maps/kalimdor.webp",
        bounds: { xMin: -6582, xMax: 10772, yMin: -11578, yMax: 5859 },
    },
    {
        mapId: 530,
        name: "Outland",
        image: "/maps/outland.webp",
        bounds: { xMin: -4592, xMax: 4592, yMin: -7549, yMax: 1620 },
    },
    {
        mapId: 571,
        name: "Northrend",
        image: "/maps/northrend.webp",
        bounds: { xMin: -3837, xMax: 7858, yMin: -9119, yMax: 2731 },
    },
];

export function getContinent(mapId: number): ContinentDef | undefined {
    return CONTINENTS.find((c) => c.mapId === mapId);
}

export function project(
    worldX: number,
    worldY: number,
    bounds: MapBounds,
    imageW: number,
    imageH: number,
): { px: number; py: number } {
    const px = ((bounds.yMax - worldY) / (bounds.yMax - bounds.yMin)) * imageW;
    const py = ((bounds.xMax - worldX) / (bounds.xMax - bounds.xMin)) * imageH;
    return { px, py };
}
