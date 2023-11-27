export function vec2(x, y) {
    return { x, y }
}

export function hexToVec4(hex, alpha = 1) {
    // Remove # if present
    hex = hex.replace('#', '');

    // Handle short hex notation (e.g., #FFF)
    if (hex.length === 3) {
        hex = hex
            .split('')
            .map(char => char + char)
            .join('');
    }

    // Parse hex values to decimal
    const r = parseInt(hex.substring(0, 2), 16);
    const g = parseInt(hex.substring(2, 4), 16);
    const b = parseInt(hex.substring(4, 6), 16);

    // Ensure alpha is between 0 and 1
    const clampedAlpha = Math.min(Math.max(alpha, 0), 1);

    return vec4(r / 255, g / 255, b / 255, clampedAlpha);
}

export function vec4(x, y, z, w) {
    return { x, y, z, w }
}

export function rect(x1, y1, x2, y2) {
    if (x2 === undefined && OP.isVec2(x1) && OP.isVec2(y1)) return rect(x1.x, x1.y, y1.x, y1.y)

    return {
        min: vec2(x1, y1),
        max: vec2(x2, y2),
    }
}

export function toU32(r, g, b, a) {
    return (a << 24) | (b << 16) | (g << 8) | r;
}

export function vec4ToU32(vec4) {
    return toU32(Math.floor(vec4.x * 255), Math.floor(vec4.y * 255),  Math.floor(vec4.z * 255),  Math.floor(vec4.w * 255));
}

export function itemSize(size, offset = 0) {
    imgui.itemSize(OP.add(size, { x: 0, y: offset }), -1.0);
}

export function getBasePos() {
    return OP.add(imgui.getWindowPos(),
        OP.sub(imgui.getCursorPos(), { x: 0, y: imgui.getScrollY() })
    );
}

export const OP = {
    isVec2: (a) => a.x !== undefined && a.y !== undefined,
    isVec4: (a) => a.x !== undefined && a.y !== undefined && a.z !== undefined && a.w !== undefined,
    isRect: (a) => a.min !== undefined && a.max !== undefined && OP.isVec2(a.min) && OP.isVec2(a.max),

    //
    // Simple Operations
    //
    mult(a, b) {
        if (OP.isVec2(a) && typeof b === "number") {
            return vec2(a.x * b, a.y * b)
        }
        if (OP.isVec4(a) && typeof b === "number") {
            return vec4(a.x * b, a.y * b, a.z * b, a.w * b)
        }
        if (OP.isVec2(a) && OP.isVec2(b)) {
            return vec2(a.x * b.x, a.y * b.y)
        }
        if (OP.isVec4(a) && OP.isVec4(b)) {
            return vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w)
        }
        throw new Error("Cannot multiply non-vec2/4")
    },
    add(a, b) {
        if (OP.isVec2(a) && OP.isVec2(b)) {
            return vec2(a.x + b.x, a.y + b.y)
        }
        if (OP.isVec4(a) && OP.isVec4(b)) {
            return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w)
        }
        throw new Error("Cannot add non-vec2/4")
    },
    sub(a, b) {
        if (OP.isVec2(a) && OP.isVec2(b)) {
            return vec2(a.x - b.x, a.y - b.y)
        }
        if (OP.isVec4(a) && OP.isVec4(b)) {
            return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w)
        }
        throw new Error("Cannot subtract non-vec2/4")
    },
    div(a, b) {
        if (OP.isVec2(a) && OP.isVec2(b)) {
            return vec2(a.x / b.x, a.y / b.y)
        }
        if (OP.isVec4(a) && OP.isVec4(b)) {
            return vec4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w)
        }
        throw new Error("Cannot divide non-vec2/4")
    },
    center(rect) {
        if (!OP.isRect(rect)) throw new Error("Cannot get center of non-rect")
        return OP.mult(OP.add(rect.min, rect.max), 0.5)
    },
    height(rect) {
        if (!OP.isRect(rect)) throw new Error("Cannot get height of non-rect")
        return rect.max.y - rect.min.y
    },
    width(rect) {
        if (!OP.isRect(rect)) throw new Error("Cannot get width of non-rect")
        return rect.max.x - rect.min.x
    },
    tl(rect) {
        if (!OP.isRect(rect)) throw new Error("Cannot get top-left of non-rect")
        return rect.min
    },
    tr(rect) {
        if (!OP.isRect(rect)) throw new Error("Cannot get top-right of non-rect")
        return vec2(rect.max.x, rect.min.y)
    },
    bl(rect) {
        if (!OP.isRect(rect)) throw new Error("Cannot get bottom-left of non-rect")
        return vec2(rect.min.x, rect.max.y)
    },
    br(rect) {
        if (!OP.isRect(rect)) throw new Error("Cannot get bottom-right of non-rect")
        return rect.max
    },
}