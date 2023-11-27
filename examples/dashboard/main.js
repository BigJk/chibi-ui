import './style.js';
import { getFont } from "./fonts.js";
import { vec2, vec4, rect, OP, toU32, getBasePos, itemSize, hexToVec4, vec4ToU32 } from "./utils.js";

let elapsed= 0;

setWindowTitle("Dashboard");
setWindowSize(800, 400);

const FONT_BIG = getFont('Ubuntu', 70);
const FONT_MEDIUM = getFont('Ubuntu', 40);
const FONT_DEFAULT = getFont('Ubuntu', 25);
const ICONS_BIG = getFont('Icons', 70);
const ICONS_MEDIUM = getFont('Icons', 40);

imgui.setCurrentFont(FONT_DEFAULT);

function createValueIcon(fontValue, fontIcon, fontDesc) {
    return (value, desc, icon, iconColor, height = 80, width = 150) => {
        const basePos = getBasePos();
        const bb = rect(basePos, OP.add(basePos, vec2(width, height)));
        const center = OP.center(bb);

        let sizeValue = null;
        let sizeIcon = null;

        imgui.pushFont(fontValue);
        {
            sizeValue = imgui.calcTextSizeSimple(value);
        }
        imgui.popFont();

        imgui.pushFont(fontIcon);
        {
            sizeIcon = imgui.calcTextSizeSimple(icon);
        }
        imgui.popFont();

        imgui.pushFont(fontValue);
        {
            imgui.dlAddText(2, vec2(center.x - (sizeValue.x + sizeIcon.x + 10) / 2, bb.min.y), toU32(255, 255, 255, 255), value);
        }
        imgui.popFont();

        imgui.pushFont(fontIcon);
        {
            imgui.dlAddText(2, vec2(center.x - (sizeValue.x + sizeIcon.x + 10) / 2 + sizeValue.x + 10, bb.min.y), vec4ToU32(iconColor), icon);
        }
        imgui.popFont();

        imgui.pushFont(fontDesc);
        {
            const size = imgui.calcTextSizeSimple(desc);
            const pos =
                OP.add(
                    vec2(OP.center(bb).x, basePos.y),
                    vec2(-(size.x / 2), height - size.y)
                );
            imgui.dlAddText(2, pos, toU32(255, 255, 255, 100), desc);
        }
        imgui.popFont();

        itemSize(vec2(width, height), 10)
    }
}

const valueIcon = createValueIcon(FONT_MEDIUM, ICONS_MEDIUM, FONT_DEFAULT);

globalThis.tick = () => {
    let now = new Date();

    elapsed += dt;

    const dw = imgui.getDisplayWidth();
    const dh = imgui.getDisplayHeight();

    imgui.setNextWindowSize(vec2(dw, dh), imgui.enums.Cond_Always);
    imgui.setNextWindowPos(vec2(0, 0), imgui.enums.Cond_Always, vec2(0, 0));
    imgui.begin('Hello World', imgui.enums.WindowFlags_NoTitleBar | imgui.enums.WindowFlags_NoResize);
    {
        imgui.pushFont(ICONS_BIG);
        {
            imgui.textColored(hexToVec4('#6faff1'), "\uE13F");
        }
        imgui.popFont();
        imgui.sameLine(0, -1.0);
        imgui.pushFont(FONT_BIG);
        {
            imgui.text(`${now.getHours().toString().padStart(2, '0')}:${now.getMinutes().toString().padStart(2, '0')}:${now.getSeconds().toString().padStart(2, '0')}`);
        }
        imgui.popFont();

        imgui.pushFont(FONT_MEDIUM);
        {
            imgui.textColored(vec4(1, 1, 1, 0.5), `${now.toLocaleDateString()}`);
        }
        imgui.popFont();

        imgui.setCursorPosY(imgui.getCursorPosY() + 120);

        valueIcon('12.3', 'CPU Usage', "\uE042", hexToVec4('#ffc94c'));
        imgui.sameLine(0, -1.0);
        imgui.setCursorPosX(imgui.getCursorPosX() + 60);
        valueIcon('284', 'File Count', "\uE07D", hexToVec4('#ffc94c'));
        imgui.sameLine(0, -1.0);
        imgui.setCursorPosX(imgui.getCursorPosX() + 60);
        valueIcon('42', 'Messages', "\uE0A5", hexToVec4('#ffc94c'));
    }
    imgui.end();

    //imgui.showMetricsWindow();
}