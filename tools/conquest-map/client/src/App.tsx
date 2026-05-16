import { useCallback, useEffect, useMemo, useState } from "react";
import { CONTINENTS } from "./projection";
import { MapCanvas } from "./MapCanvas";
import { Dashboard } from "./Dashboard";
import { fetchAll } from "./api";
import { FACTION_COLOR, FACTION_LABEL, type Banner, type Waypoint } from "./types";
import {
    DEFAULT_CALIBRATION,
    loadCalibrations,
    saveCalibrations,
    type Calibration,
} from "./calibration";

const REFRESH_MS = 10_000;

type ViewMode = "map" | "dashboard";

export default function App() {
    const [view, setView] = useState<ViewMode>("map");
    const [mapId, setMapId] = useState<number>(CONTINENTS[0].mapId);
    const [banners, setBanners] = useState<Banner[]>([]);
    const [waypoints, setWaypoints] = useState<Waypoint[]>([]);
    const [autoRefresh, setAutoRefresh] = useState(true);
    const [status, setStatus] = useState<string>("…");
    const [lastUpdate, setLastUpdate] = useState<Date | null>(null);
    const [calibrations, setCalibrations] = useState<
        Record<number, Calibration>
    >(() => loadCalibrations());
    const [showCal, setShowCal] = useState(false);

    const load = useCallback(async () => {
        try {
            setStatus("chargement…");
            const data = await fetchAll();
            setBanners(data.banners);
            setWaypoints(data.waypoints);
            setLastUpdate(new Date());
            setStatus(
                `${data.banners.length} bannières · ${data.waypoints.length} waypoints`,
            );
        } catch (err) {
            setStatus(`erreur: ${(err as Error).message}`);
        }
    }, []);

    useEffect(() => {
        if (view !== "map") return;
        void load();
    }, [load, view]);

    useEffect(() => {
        if (view !== "map" || !autoRefresh) return;
        const id = window.setInterval(load, REFRESH_MS);
        return () => window.clearInterval(id);
    }, [autoRefresh, load, view]);

    useEffect(() => {
        saveCalibrations(calibrations);
    }, [calibrations]);

    const continent = CONTINENTS.find((c) => c.mapId === mapId) ?? CONTINENTS[0];
    const calibration = useMemo<Calibration>(
        () => calibrations[mapId] ?? DEFAULT_CALIBRATION,
        [calibrations, mapId],
    );

    const patchCal = (patch: Partial<Calibration>) => {
        setCalibrations((prev) => ({
            ...prev,
            [mapId]: { ...(prev[mapId] ?? DEFAULT_CALIBRATION), ...patch },
        }));
    };

    const resetCal = () => {
        setCalibrations((prev) => {
            const next = { ...prev };
            delete next[mapId];
            return next;
        });
    };

    return (
        <div className="app">
            <header className="toolbar">
                <h1>Conquest Map</h1>

                <div className="tabs view-switch">
                    <button
                        className={`tab${view === "map" ? " active" : ""}`}
                        onClick={() => setView("map")}
                    >
                        🗺 Carte
                    </button>
                    <button
                        className={`tab${view === "dashboard" ? " active" : ""}`}
                        onClick={() => setView("dashboard")}
                    >
                        📊 Dashboard
                    </button>
                </div>

                {view === "map" && (
                    <>
                        <div className="tabs">
                            {CONTINENTS.map((c) => (
                                <button
                                    key={c.mapId}
                                    className={`tab${c.mapId === mapId ? " active" : ""}`}
                                    onClick={() => setMapId(c.mapId)}
                                >
                                    {c.name}
                                </button>
                            ))}
                        </div>
                        <button className="tab" onClick={load}>
                            ↻ refresh
                        </button>
                        <button
                            className={`tab${showCal ? " active" : ""}`}
                            onClick={() => setShowCal((v) => !v)}
                        >
                            ⚙ calibration
                        </button>
                    </>
                )}

                <label className="toggle">
                    <input
                        type="checkbox"
                        checked={autoRefresh}
                        onChange={(e) => setAutoRefresh(e.target.checked)}
                    />
                    auto-refresh (10s)
                </label>

                {view === "map" && (
                    <div className="legend">
                        {[1, 2, 0].map((f) => (
                            <span key={f}>
                                <span
                                    className="dot"
                                    style={{ background: FACTION_COLOR[f] }}
                                />
                                {FACTION_LABEL[f]}
                            </span>
                        ))}
                    </div>
                )}

                {view === "map" && (
                    <div className="status">
                        {status}
                        {lastUpdate && ` · maj ${lastUpdate.toLocaleTimeString()}`}
                    </div>
                )}
            </header>

            {view === "map" && showCal && (
                <div className="cal-bar">
                    <span className="cal-title">
                        Calibration · {continent.name}
                    </span>
                    <CalSlider
                        label="scale"
                        min={0.3}
                        max={2.5}
                        step={0.005}
                        value={calibration.scale}
                        onChange={(v) => patchCal({ scale: v })}
                    />
                    <CalSlider
                        label="offset X"
                        min={-0.5}
                        max={0.5}
                        step={0.001}
                        value={calibration.offsetX}
                        onChange={(v) => patchCal({ offsetX: v })}
                    />
                    <CalSlider
                        label="offset Y"
                        min={-0.5}
                        max={0.5}
                        step={0.001}
                        value={calibration.offsetY}
                        onChange={(v) => patchCal({ offsetY: v })}
                    />
                    <button className="tab" onClick={resetCal}>
                        reset
                    </button>
                    <span className="cal-hint">
                        sauvegardé localement par continent
                    </span>
                </div>
            )}

            {view === "map" ? (
                <MapCanvas
                    continent={continent}
                    banners={banners}
                    waypoints={waypoints}
                    calibration={calibration}
                />
            ) : (
                <Dashboard autoRefresh={autoRefresh} />
            )}
        </div>
    );
}

function CalSlider({
    label,
    min,
    max,
    step,
    value,
    onChange,
}: {
    label: string;
    min: number;
    max: number;
    step: number;
    value: number;
    onChange: (v: number) => void;
}) {
    return (
        <label className="cal-slider">
            <span className="cal-label">{label}</span>
            <input
                type="range"
                min={min}
                max={max}
                step={step}
                value={value}
                onChange={(e) => onChange(Number(e.target.value))}
            />
            <input
                type="number"
                min={min}
                max={max}
                step={step}
                value={value}
                onChange={(e) => onChange(Number(e.target.value))}
                className="cal-number"
            />
        </label>
    );
}
