import type { Banner, Waypoint } from "./types";
import type { DashboardData } from "./dashboard-types";

export async function fetchAll(): Promise<{
    banners: Banner[];
    waypoints: Waypoint[];
}> {
    const res = await fetch("/api/all");
    if (!res.ok) throw new Error(`/api/all returned ${res.status}`);
    return res.json();
}

export async function fetchDashboard(): Promise<DashboardData> {
    const res = await fetch("/api/dashboard");
    if (!res.ok) throw new Error(`/api/dashboard returned ${res.status}`);
    return res.json();
}
