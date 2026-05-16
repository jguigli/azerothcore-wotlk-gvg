export type Calibration = {
    // Uniform scale around viewBox center. 1 = no change.
    scale: number;
    // Translation in fraction of viewBox width / height. 0 = no shift.
    offsetX: number;
    offsetY: number;
};

export const DEFAULT_CALIBRATION: Calibration = {
    scale: 1,
    offsetX: 0,
    offsetY: 0,
};

const STORAGE_KEY = "conquest-map-calibrations-v1";

export function loadCalibrations(): Record<number, Calibration> {
    try {
        const raw = localStorage.getItem(STORAGE_KEY);
        if (!raw) return {};
        const parsed = JSON.parse(raw) as Record<string, Calibration>;
        const out: Record<number, Calibration> = {};
        for (const [k, v] of Object.entries(parsed)) {
            out[Number(k)] = {
                scale: typeof v.scale === "number" ? v.scale : 1,
                offsetX: typeof v.offsetX === "number" ? v.offsetX : 0,
                offsetY: typeof v.offsetY === "number" ? v.offsetY : 0,
            };
        }
        return out;
    } catch {
        return {};
    }
}

export function saveCalibrations(map: Record<number, Calibration>): void {
    try {
        localStorage.setItem(STORAGE_KEY, JSON.stringify(map));
    } catch {
        /* quota / disabled — ignore */
    }
}

export function applyCalibration(
    px: number,
    py: number,
    cal: Calibration,
    vbW: number,
    vbH: number,
): { px: number; py: number } {
    const cx = vbW / 2;
    const cy = vbH / 2;
    return {
        px: (px - cx) * cal.scale + cx + cal.offsetX * vbW,
        py: (py - cy) * cal.scale + cy + cal.offsetY * vbH,
    };
}
