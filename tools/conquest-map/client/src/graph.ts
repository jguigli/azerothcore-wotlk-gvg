import type { Waypoint } from "./types";

const MAX_EDGE_DISTANCE = 500; // yards, same rule as ConquestWaypointMgr (server)

export type Edge = { a: Waypoint; b: Waypoint; distance: number };

export function buildEdges(waypoints: Waypoint[]): Edge[] {
    const edges: Edge[] = [];
    for (let i = 0; i < waypoints.length; i++) {
        for (let j = i + 1; j < waypoints.length; j++) {
            const a = waypoints[i];
            const b = waypoints[j];
            if (a.map !== b.map) continue;
            const dx = a.x - b.x;
            const dy = a.y - b.y;
            const d = Math.hypot(dx, dy);
            if (d <= MAX_EDGE_DISTANCE) {
                edges.push({ a, b, distance: d });
            }
        }
    }
    return edges;
}
