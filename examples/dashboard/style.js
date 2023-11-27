import { toU32 } from "./utils.js";

//
// Style Colors
//
imgui.pushStyleColor(imgui.enums.Col_Border, toU32(255, 255, 255, 100));
imgui.pushStyleColor(imgui.enums.Col_WindowBg, toU32(31, 31, 31, 255));
imgui.pushStyleColor(imgui.enums.Col_ScrollbarBg, toU32(0, 0, 0, 0));
imgui.pushStyleColor(imgui.enums.Col_ScrollbarGrab, toU32(255, 255, 255, 30));
imgui.pushStyleColor(imgui.enums.Col_ScrollbarGrabActive, toU32(255, 255, 255, 40));
imgui.pushStyleColor(imgui.enums.Col_ScrollbarGrabHovered, toU32(255, 255, 255, 40));
imgui.pushStyleColor(imgui.enums.Col_FrameBg, toU32(46, 46, 46, 255));
imgui.pushStyleColor(imgui.enums.Col_TextSelectedBg, toU32(255, 255, 255, 20));
imgui.pushStyleColor(imgui.enums.Col_PopupBg, toU32(42, 44, 45, 255));

//
// Style Variables
//
imgui.pushStyleVarFloat(imgui.enums.StyleVar_WindowRounding, 0);
imgui.pushStyleVarFloat(imgui.enums.StyleVar_WindowBorderSize, 1);
imgui.pushStyleVarFloat(imgui.enums.StyleVar_ScrollbarSize, 2);
imgui.pushStyleVarFloat(imgui.enums.StyleVar_PopupBorderSize, 1);
imgui.pushStyleVarVec2(imgui.enums.StyleVar_FramePadding, { x: 0, y: 0 });
imgui.pushStyleVarVec2(imgui.enums.StyleVar_WindowPadding, { x: 40, y: 40 });

//
// Background Color
//
imgui.setBackgroundColorU32(toU32(31, 32, 41, 255));