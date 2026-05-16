import { useEffect, useMemo, useRef, useState } from "react";
import type { Banner, Waypoint } from "./types";
import { FACTION_COLOR, FACTION_LABEL } from "./types";
import { type ContinentDef, project } from "./projection";
import { buildEdges } from "./graph";
import { applyCalibration, type Calibration } from "./calibration";

type Props = {
    continent: ContinentDef;
    banners: Banner[];
    waypoints: Waypoint[];
    calibration: Calibration;
};

type HoverState = {
    banner: Banner;
    clientX: number;
    clientY: number;
};

type ImageState =
    | { kind: "loading" }
    | { kind: "ok"; naturalW: number; naturalH: number }
    | { kind: "missing" };

function boundsAspect(c: ContinentDef): number {
    const w = c.bounds.yMax - c.bounds.yMin;
    const h = c.bounds.xMax - c.bounds.xMin;
    return w / h;
}

const BANNER_PX = 9;
const WAYPOINT_PX = 3;
const EDGE_PX = 1.6;

export function MapCanvas({ continent, banners, waypoints, calibration }: Props) {
    const svgRef = useRef<SVGSVGElement>(null);
    const [imageState, setImageState] = useState<ImageState>({
        kind: "loading",
    });
    const [hover, setHover] = useState<HoverState | null>(null);
    const [scale, setScale] = useState(1);

    useEffect(() => {
        setImageState({ kind: "loading" });
        const img = new Image();
        img.onload = () =>
            setImageState({
                kind: "ok",
                naturalW: img.naturalWidth,
                naturalH: img.naturalHeight,
            });
        img.onerror = () => setImageState({ kind: "missing" });
        img.src = continent.image;
    }, [continent.image]);

    const vbW = imageState.kind === "ok" ? imageState.naturalW : 1000;
    const vbH =
        imageState.kind === "ok"
            ? imageState.naturalH
            : Math.round(1000 / boundsAspect(continent));

    useEffect(() => {
        const el = svgRef.current;
        if (!el) return;
        const update = () => {
            const rect = el.getBoundingClientRect();
            if (rect.width === 0 || rect.height === 0) return;
            const s = Math.min(rect.width / vbW, rect.height / vbH);
            if (s > 0) setScale(s);
        };
        update();
        const ro = new ResizeObserver(update);
        ro.observe(el);
        return () => ro.disconnect();
    }, [vbW, vbH]);

    const continentBanners = useMemo(
        () => banners.filter((b) => b.map === continent.mapId),
        [banners, continent.mapId],
    );
    const continentWaypoints = useMemo(
        () => waypoints.filter((w) => w.map === continent.mapId),
        [waypoints, continent.mapId],
    );
    const edges = useMemo(
        () => buildEdges(continentWaypoints),
        [continentWaypoints],
    );

    const projectCal = (worldX: number, worldY: number) => {
        const p = project(worldX, worldY, continent.bounds, vbW, vbH);
        return applyCalibration(p.px, p.py, calibration, vbW, vbH);
    };

    const bannerR = BANNER_PX / scale;
    const waypointR = WAYPOINT_PX / scale;
    const edgeW = EDGE_PX / scale;

    return (
        <div className="canvas-wrap">
            <svg
                ref={svgRef}
                className="stage-svg"
                viewBox={`0 0 ${vbW} ${vbH}`}
                preserveAspectRatio="xMidYMid meet"
            >
                {imageState.kind === "ok" ? (
                    <image
                        href={continent.image}
                        x={0}
                        y={0}
                        width={vbW}
                        height={vbH}
                        opacity={0.9}
                    />
                ) : (
                    <>
                        <rect x={0} y={0} width={vbW} height={vbH} fill="#161922" />
                        {Array.from({ length: 24 }).map((_, i) => (
                            <line
                                key={`gx-${i}`}
                                x1={(i * vbW) / 24}
                                y1={0}
                                x2={(i * vbW) / 24}
                                y2={vbH}
                                stroke="rgba(255,255,255,0.04)"
                            />
                        ))}
                        {Array.from({ length: 24 }).map((_, i) => (
                            <line
                                key={`gy-${i}`}
                                x1={0}
                                y1={(i * vbH) / 24}
                                x2={vbW}
                                y2={(i * vbH) / 24}
                                stroke="rgba(255,255,255,0.04)"
                            />
                        ))}
                    </>
                )}

                {edges.map((e, i) => {
                    const pa = projectCal(e.a.x, e.a.y);
                    const pb = projectCal(e.b.x, e.b.y);
                    return (
                        <line
                            key={i}
                            x1={pa.px}
                            y1={pa.py}
                            x2={pb.px}
                            y2={pb.py}
                            stroke="rgba(255, 200, 80, 0.55)"
                            strokeWidth={edgeW}
                        />
                    );
                })}

                {continentWaypoints.map((wp) => {
                    const p = projectCal(wp.x, wp.y);
                    return (
                        <circle
                            key={`wp-${wp.guid}`}
                            cx={p.px}
                            cy={p.py}
                            r={waypointR}
                            fill="rgba(255, 200, 80, 0.9)"
                        />
                    );
                })}

                {continentBanners.map((b) => {
                    const p = projectCal(b.x, b.y);
                    return (
                        <circle
                            key={`b-${b.guid}`}
                            className="banner-marker"
                            cx={p.px}
                            cy={p.py}
                            r={bannerR}
                            fill={FACTION_COLOR[b.faction] ?? FACTION_COLOR[0]}
                            stroke="#0f1115"
                            strokeWidth={bannerR / 3}
                            onMouseEnter={(ev) =>
                                setHover({
                                    banner: b,
                                    clientX: ev.clientX,
                                    clientY: ev.clientY,
                                })
                            }
                            onMouseMove={(ev) =>
                                setHover({
                                    banner: b,
                                    clientX: ev.clientX,
                                    clientY: ev.clientY,
                                })
                            }
                            onMouseLeave={() => setHover(null)}
                        />
                    );
                })}
            </svg>

            {continentBanners.length === 0 && (
                <div className="empty">
                    Aucune bannière 400010 sur {continent.name}
                </div>
            )}

            {hover && (
                <div
                    className="tooltip"
                    style={{
                        left: hover.clientX + 12,
                        top: hover.clientY + 12,
                    }}
                >
                    <dl>
                        <dt>guid</dt>
                        <dd>{hover.banner.guid}</dd>
                        <dt>faction</dt>
                        <dd>{FACTION_LABEL[hover.banner.faction] ?? "?"}</dd>
                        <dt>guild</dt>
                        <dd>{hover.banner.guildId || "—"}</dd>
                        <dt>zone</dt>
                        <dd>
                            {hover.banner.zoneId} / area {hover.banner.areaId}
                        </dd>
                        <dt>pos</dt>
                        <dd>
                            {hover.banner.x.toFixed(0)},{" "}
                            {hover.banner.y.toFixed(0)},{" "}
                            {hover.banner.z.toFixed(0)}
                        </dd>
                    </dl>
                </div>
            )}
        </div>
    );
}
