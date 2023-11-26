#include "js_imgui.h"

#include <tuple>

#include "./imgui.h"
#include "./imgui_internal.h"

#define JS_CALL_VOID(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    FN; \
    return JS_UNDEFINED; \
}, "", 0));               \

#define JS_CALL_STRING(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    const auto arg = JS_ToCString(ctx, argv[0]); \
    FN; \
    JS_FreeCString(ctx, arg); \
    return JS_UNDEFINED; \
}, "", 0)); \

#define JS_CALL_FLOAT(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    double arg = 0.0; \
    JS_ToFloat64(ctx, &arg, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, "", 0)); \

#define JS_CALL_VEC2(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    const auto arg = js_to_imvec2(ctx, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, "", 0)); \

#define JS_CALL_VEC4(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    const auto arg = js_to_imvec4(ctx, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, "", 0)); \

#define JS_CALL_BOOL(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    bool arg = JS_ToBool(ctx, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, "", 0)); \

#define JS_CALL_UINT32(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    uint32_t arg = 0; \
    JS_ToUint32(ctx, &arg, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, "", 0)); \

#define JS_CALL_INT32(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    int32_t arg = 0; \
    JS_ToInt32(ctx, &arg, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, "", 0));                     \

#define JS_FUNC(NAME, FN_BODY) \
JS_SetProperty(ctx, imgui_obj, JS_NewAtom(ctx, NAME), JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue FN_BODY, "", 0)); \

#define JS_ENSURE_ARGS(N) if (argc < N) { return JS_EXCEPTION; }
#define JS_ARG(NAME, TYPE, JS_TYPE, N) TYPE NAME = 0; JS_To##JS_TYPE(ctx, &NAME, argv[N]);
#define JS_ARG_BOOL(NAME, N) bool NAME = JS_ToBool(ctx, argv[N]);
#define JS_ARG_VEC2(NAME, N) const auto NAME = js_to_imvec2(ctx, argv[N]);
#define JS_ARG_VEC4(NAME, N) const auto NAME = js_to_imvec4(ctx, argv[N]);
#define JS_ARG_STRING(NAME, N) const auto NAME = JS_ToCString(ctx, argv[N]);

ImVec2 js_to_imvec2(JSContext* ctx, JSValueConst val) {
    double dx = 0.0;
    double dy = 0.0;
    JSValue x = JS_GetPropertyStr(ctx, val, "x");
    JSValue y = JS_GetPropertyStr(ctx, val, "y");
    JS_ToFloat64(ctx, &dx, x);
    JS_ToFloat64(ctx, &dy, y);
    JS_FreeValue(ctx, x);
    JS_FreeValue(ctx, y);
    return ImVec2(dx, dy);
}

ImVec4 js_to_imvec4(JSContext* ctx, JSValueConst val) {
    double dx = 0.0;
    double dy = 0.0;
    double dz = 0.0;
    double dw = 0.0;
    JSValue x = JS_GetPropertyStr(ctx, val, "x");
    JSValue y = JS_GetPropertyStr(ctx, val, "y");
    JSValue z = JS_GetPropertyStr(ctx, val, "z");
    JSValue w = JS_GetPropertyStr(ctx, val, "w");
    JS_ToFloat64(ctx, &dx, x);
    JS_ToFloat64(ctx, &dy, y);
    JS_ToFloat64(ctx, &dz, z);
    JS_ToFloat64(ctx, &dw, w);
    JS_FreeValue(ctx, x);
    JS_FreeValue(ctx, y);
    JS_FreeValue(ctx, z);
    JS_FreeValue(ctx, w);
    return ImVec4(dx, dy, dz, dw);
}

JSValue js_from_imvec2(JSContext* ctx, ImVec2 vec) {
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, vec.x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, vec.y));
    return obj;
}

JSValue js_from_imvec4(JSContext* ctx, ImVec4 vec) {
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, vec.x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, vec.y));
    JS_SetPropertyStr(ctx, obj, "z", JS_NewFloat64(ctx, vec.z));
    JS_SetPropertyStr(ctx, obj, "w", JS_NewFloat64(ctx, vec.w));
    return obj;
}

void js_imgui_enums(JSContext* ctx, JSValue imgui_obj) {
    // ImGuiCond_
    JS_SetPropertyStr(ctx, imgui_obj, "Cond_None", JS_NewInt32(ctx, ImGuiCond_None));
    JS_SetPropertyStr(ctx, imgui_obj, "Cond_Always", JS_NewInt32(ctx, ImGuiCond_Always));
    JS_SetPropertyStr(ctx, imgui_obj, "Cond_Once", JS_NewInt32(ctx, ImGuiCond_Once));
    JS_SetPropertyStr(ctx, imgui_obj, "Cond_FirstUseEver", JS_NewInt32(ctx, ImGuiCond_FirstUseEver));
    JS_SetPropertyStr(ctx, imgui_obj, "Cond_Appearing", JS_NewInt32(ctx, ImGuiCond_Appearing));

    // ImGuiWindowFlags_
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_None", JS_NewInt32(ctx, ImGuiWindowFlags_None));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoTitleBar", JS_NewInt32(ctx, ImGuiWindowFlags_NoTitleBar));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoResize", JS_NewInt32(ctx, ImGuiWindowFlags_NoResize));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoMove", JS_NewInt32(ctx, ImGuiWindowFlags_NoMove));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoScrollbar", JS_NewInt32(ctx, ImGuiWindowFlags_NoScrollbar));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoScrollWithMouse", JS_NewInt32(ctx, ImGuiWindowFlags_NoScrollWithMouse));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoCollapse", JS_NewInt32(ctx, ImGuiWindowFlags_NoCollapse));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_AlwaysAutoResize", JS_NewInt32(ctx, ImGuiWindowFlags_AlwaysAutoResize));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoBackground", JS_NewInt32(ctx, ImGuiWindowFlags_NoBackground));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoSavedSettings", JS_NewInt32(ctx, ImGuiWindowFlags_NoSavedSettings));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoMouseInputs", JS_NewInt32(ctx, ImGuiWindowFlags_NoMouseInputs));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_MenuBar", JS_NewInt32(ctx, ImGuiWindowFlags_MenuBar));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_HorizontalScrollbar", JS_NewInt32(ctx, ImGuiWindowFlags_HorizontalScrollbar));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoFocusOnAppearing", JS_NewInt32(ctx, ImGuiWindowFlags_NoFocusOnAppearing));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoBringToFrontOnFocus", JS_NewInt32(ctx, ImGuiWindowFlags_NoBringToFrontOnFocus));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_AlwaysVerticalScrollbar", JS_NewInt32(ctx, ImGuiWindowFlags_AlwaysVerticalScrollbar));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_AlwaysHorizontalScrollbar", JS_NewInt32(ctx, ImGuiWindowFlags_AlwaysHorizontalScrollbar));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_AlwaysUseWindowPadding", JS_NewInt32(ctx, ImGuiWindowFlags_AlwaysUseWindowPadding));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoNavInputs", JS_NewInt32(ctx, ImGuiWindowFlags_NoNavInputs));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoNavFocus", JS_NewInt32(ctx, ImGuiWindowFlags_NoNavFocus));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_UnsavedDocument", JS_NewInt32(ctx, ImGuiWindowFlags_UnsavedDocument));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoNav", JS_NewInt32(ctx, ImGuiWindowFlags_NoNav));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoDecoration", JS_NewInt32(ctx, ImGuiWindowFlags_NoDecoration));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoInputs", JS_NewInt32(ctx, ImGuiWindowFlags_NoInputs));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NavFlattened", JS_NewInt32(ctx, ImGuiWindowFlags_NavFlattened));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_ChildWindow", JS_NewInt32(ctx, ImGuiWindowFlags_ChildWindow));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_Tooltip", JS_NewInt32(ctx, ImGuiWindowFlags_Tooltip));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_Popup", JS_NewInt32(ctx, ImGuiWindowFlags_Popup));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_Modal", JS_NewInt32(ctx, ImGuiWindowFlags_Modal));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_ChildMenu", JS_NewInt32(ctx, ImGuiWindowFlags_ChildMenu));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoNavFocus", JS_NewInt32(ctx, ImGuiWindowFlags_NoNavFocus));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoNavFocus", JS_NewInt32(ctx, ImGuiWindowFlags_NoNavFocus));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoNavFocus", JS_NewInt32(ctx, ImGuiWindowFlags_NoNavFocus));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoNavFocus", JS_NewInt32(ctx, ImGuiWindowFlags_NoNavFocus));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoNavFocus", JS_NewInt32(ctx, ImGuiWindowFlags_NoNavFocus));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoNavFocus", JS_NewInt32(ctx, ImGuiWindowFlags_NoNavFocus));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoNavFocus", JS_NewInt32(ctx, ImGuiWindowFlags_NoNavFocus));
    JS_SetPropertyStr(ctx, imgui_obj, "WindowFlags_NoNavFocus", JS_NewInt32(ctx, ImGuiWindowFlags_NoNavFocus));

    // ImGuiInputTextFlags_
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_None", JS_NewInt32(ctx, ImGuiInputTextFlags_None));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CharsDecimal", JS_NewInt32(ctx, ImGuiInputTextFlags_CharsDecimal));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CharsHexadecimal", JS_NewInt32(ctx, ImGuiInputTextFlags_CharsHexadecimal));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CharsUppercase", JS_NewInt32(ctx, ImGuiInputTextFlags_CharsUppercase));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CharsNoBlank", JS_NewInt32(ctx, ImGuiInputTextFlags_CharsNoBlank));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_AutoSelectAll", JS_NewInt32(ctx, ImGuiInputTextFlags_AutoSelectAll));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_EnterReturnsTrue", JS_NewInt32(ctx, ImGuiInputTextFlags_EnterReturnsTrue));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CallbackCompletion", JS_NewInt32(ctx, ImGuiInputTextFlags_CallbackCompletion));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CallbackHistory", JS_NewInt32(ctx, ImGuiInputTextFlags_CallbackHistory));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CallbackAlways", JS_NewInt32(ctx, ImGuiInputTextFlags_CallbackAlways));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CallbackCharFilter", JS_NewInt32(ctx, ImGuiInputTextFlags_CallbackCharFilter));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_AllowTabInput", JS_NewInt32(ctx, ImGuiInputTextFlags_AllowTabInput));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CtrlEnterForNewLine", JS_NewInt32(ctx, ImGuiInputTextFlags_CtrlEnterForNewLine));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_NoHorizontalScroll", JS_NewInt32(ctx, ImGuiInputTextFlags_NoHorizontalScroll));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_ReadOnly", JS_NewInt32(ctx, ImGuiInputTextFlags_ReadOnly));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_Password", JS_NewInt32(ctx, ImGuiInputTextFlags_Password));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_NoUndoRedo", JS_NewInt32(ctx, ImGuiInputTextFlags_NoUndoRedo));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CharsScientific", JS_NewInt32(ctx, ImGuiInputTextFlags_CharsScientific));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CallbackResize", JS_NewInt32(ctx, ImGuiInputTextFlags_CallbackResize));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_CallbackEdit", JS_NewInt32(ctx, ImGuiInputTextFlags_CallbackEdit));
    JS_SetPropertyStr(ctx, imgui_obj, "InputTextFlags_Multiline", JS_NewInt32(ctx, ImGuiInputTextFlags_Multiline));

    // ImGuiTreeNodeFlags_
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_None", JS_NewInt32(ctx, ImGuiTreeNodeFlags_None));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_Selected", JS_NewInt32(ctx, ImGuiTreeNodeFlags_Selected));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_Framed", JS_NewInt32(ctx, ImGuiTreeNodeFlags_Framed));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_AllowItemOverlap", JS_NewInt32(ctx, ImGuiTreeNodeFlags_AllowItemOverlap));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_NoTreePushOnOpen", JS_NewInt32(ctx, ImGuiTreeNodeFlags_NoTreePushOnOpen));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_NoAutoOpenOnLog", JS_NewInt32(ctx, ImGuiTreeNodeFlags_NoAutoOpenOnLog));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_DefaultOpen", JS_NewInt32(ctx, ImGuiTreeNodeFlags_DefaultOpen));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_OpenOnDoubleClick", JS_NewInt32(ctx, ImGuiTreeNodeFlags_OpenOnDoubleClick));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_OpenOnArrow", JS_NewInt32(ctx, ImGuiTreeNodeFlags_OpenOnArrow));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_Leaf", JS_NewInt32(ctx, ImGuiTreeNodeFlags_Leaf));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_Bullet", JS_NewInt32(ctx, ImGuiTreeNodeFlags_Bullet));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_FramePadding", JS_NewInt32(ctx, ImGuiTreeNodeFlags_FramePadding));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_SpanAvailWidth", JS_NewInt32(ctx, ImGuiTreeNodeFlags_SpanAvailWidth));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_SpanFullWidth", JS_NewInt32(ctx, ImGuiTreeNodeFlags_SpanFullWidth));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_NavLeftJumpsBackHere", JS_NewInt32(ctx, ImGuiTreeNodeFlags_NavLeftJumpsBackHere));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_CollapsingHeader", JS_NewInt32(ctx, ImGuiTreeNodeFlags_CollapsingHeader));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_AllowOverlapMode", JS_NewInt32(ctx, ImGuiTreeNodeFlags_AllowOverlap));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_NoTreePushOnOpen", JS_NewInt32(ctx, ImGuiTreeNodeFlags_NoTreePushOnOpen));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_NoAutoOpenOnLog", JS_NewInt32(ctx, ImGuiTreeNodeFlags_NoAutoOpenOnLog));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_DefaultOpen", JS_NewInt32(ctx, ImGuiTreeNodeFlags_DefaultOpen));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_OpenOnDoubleClick", JS_NewInt32(ctx, ImGuiTreeNodeFlags_OpenOnDoubleClick));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_OpenOnArrow", JS_NewInt32(ctx, ImGuiTreeNodeFlags_OpenOnArrow));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_Leaf", JS_NewInt32(ctx, ImGuiTreeNodeFlags_Leaf));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_Bullet", JS_NewInt32(ctx, ImGuiTreeNodeFlags_Bullet));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_FramePadding", JS_NewInt32(ctx, ImGuiTreeNodeFlags_FramePadding));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_SpanAvailWidth", JS_NewInt32(ctx, ImGuiTreeNodeFlags_SpanAvailWidth));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_SpanFullWidth", JS_NewInt32(ctx, ImGuiTreeNodeFlags_SpanFullWidth));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_NavLeftJumpsBackHere", JS_NewInt32(ctx, ImGuiTreeNodeFlags_NavLeftJumpsBackHere));
    JS_SetPropertyStr(ctx, imgui_obj, "TreeNodeFlags_CollapsingHeader", JS_NewInt32(ctx, ImGuiTreeNodeFlags_CollapsingHeader));

    // ImGuiSelectableFlags_
    JS_SetPropertyStr(ctx, imgui_obj, "SelectableFlags_None", JS_NewInt32(ctx, ImGuiSelectableFlags_None));
    JS_SetPropertyStr(ctx, imgui_obj, "SelectableFlags_DontClosePopups", JS_NewInt32(ctx, ImGuiSelectableFlags_DontClosePopups));
    JS_SetPropertyStr(ctx, imgui_obj, "SelectableFlags_SpanAllColumns", JS_NewInt32(ctx, ImGuiSelectableFlags_SpanAllColumns));
    JS_SetPropertyStr(ctx, imgui_obj, "SelectableFlags_AllowDoubleClick", JS_NewInt32(ctx, ImGuiSelectableFlags_AllowDoubleClick));
    JS_SetPropertyStr(ctx, imgui_obj, "SelectableFlags_Disabled", JS_NewInt32(ctx, ImGuiSelectableFlags_Disabled));
    JS_SetPropertyStr(ctx, imgui_obj, "SelectableFlags_AllowItemOverlap", JS_NewInt32(ctx, ImGuiSelectableFlags_AllowItemOverlap));

    // ImGuiComboFlags_
    JS_SetPropertyStr(ctx, imgui_obj, "ComboFlags_None", JS_NewInt32(ctx, ImGuiComboFlags_None));
    JS_SetPropertyStr(ctx, imgui_obj, "ComboFlags_PopupAlignLeft", JS_NewInt32(ctx, ImGuiComboFlags_PopupAlignLeft));
    JS_SetPropertyStr(ctx, imgui_obj, "ComboFlags_HeightSmall", JS_NewInt32(ctx, ImGuiComboFlags_HeightSmall));
    JS_SetPropertyStr(ctx, imgui_obj, "ComboFlags_HeightRegular", JS_NewInt32(ctx, ImGuiComboFlags_HeightRegular));
    JS_SetPropertyStr(ctx, imgui_obj, "ComboFlags_HeightLarge", JS_NewInt32(ctx, ImGuiComboFlags_HeightLarge));
    JS_SetPropertyStr(ctx, imgui_obj, "ComboFlags_HeightLargest", JS_NewInt32(ctx, ImGuiComboFlags_HeightLargest));
    JS_SetPropertyStr(ctx, imgui_obj, "ComboFlags_NoArrowButton", JS_NewInt32(ctx, ImGuiComboFlags_NoArrowButton));
    JS_SetPropertyStr(ctx, imgui_obj, "ComboFlags_NoPreview", JS_NewInt32(ctx, ImGuiComboFlags_NoPreview));
    JS_SetPropertyStr(ctx, imgui_obj, "ComboFlags_HeightMask_", JS_NewInt32(ctx, ImGuiComboFlags_HeightMask_));

    // ImGuiTabBarFlags_
    JS_SetPropertyStr(ctx, imgui_obj, "TabBarFlags_None", JS_NewInt32(ctx, ImGuiTabBarFlags_None));
    JS_SetPropertyStr(ctx, imgui_obj, "TabBarFlags_Reorderable", JS_NewInt32(ctx, ImGuiTabBarFlags_Reorderable));
    JS_SetPropertyStr(ctx, imgui_obj, "TabBarFlags_AutoSelectNewTabs", JS_NewInt32(ctx, ImGuiTabBarFlags_AutoSelectNewTabs));
    JS_SetPropertyStr(ctx, imgui_obj, "TabBarFlags_TabListPopupButton", JS_NewInt32(ctx, ImGuiTabBarFlags_TabListPopupButton));
    JS_SetPropertyStr(ctx, imgui_obj, "TabBarFlags_NoCloseWithMiddleMouseButton", JS_NewInt32(ctx, ImGuiTabBarFlags_NoCloseWithMiddleMouseButton));
    JS_SetPropertyStr(ctx, imgui_obj, "TabBarFlags_NoTabListScrollingButtons", JS_NewInt32(ctx, ImGuiTabBarFlags_NoTabListScrollingButtons));
    JS_SetPropertyStr(ctx, imgui_obj, "TabBarFlags_NoTooltip", JS_NewInt32(ctx, ImGuiTabBarFlags_NoTooltip));
    JS_SetPropertyStr(ctx, imgui_obj, "TabBarFlags_FittingPolicyResizeDown", JS_NewInt32(ctx, ImGuiTabBarFlags_FittingPolicyResizeDown));
    JS_SetPropertyStr(ctx, imgui_obj, "TabBarFlags_FittingPolicyScroll", JS_NewInt32(ctx, ImGuiTabBarFlags_FittingPolicyScroll));
    JS_SetPropertyStr(ctx, imgui_obj, "TabBarFlags_FittingPolicyMask_", JS_NewInt32(ctx, ImGuiTabBarFlags_FittingPolicyMask_));
    JS_SetPropertyStr(ctx, imgui_obj, "TabBarFlags_FittingPolicyDefault_", JS_NewInt32(ctx, ImGuiTabBarFlags_FittingPolicyDefault_));

    // DrawListType_
    JS_SetPropertyStr(ctx, imgui_obj, "DrawListType_Foreground", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, imgui_obj, "DrawListType_Background", JS_NewInt32(ctx, 1));
    JS_SetPropertyStr(ctx, imgui_obj, "DrawListType_Window", JS_NewInt32(ctx, 2));

    // ImDrawFlags_
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_None", JS_NewInt32(ctx, ImDrawFlags_None));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_Closed", JS_NewInt32(ctx, ImDrawFlags_Closed));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersTopLeft", JS_NewInt32(ctx, ImDrawFlags_RoundCornersTopLeft));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersTopRight", JS_NewInt32(ctx, ImDrawFlags_RoundCornersTopRight));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersBottomLeft", JS_NewInt32(ctx, ImDrawFlags_RoundCornersBottomLeft));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersBottomRight", JS_NewInt32(ctx, ImDrawFlags_RoundCornersBottomRight));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersNone", JS_NewInt32(ctx, ImDrawFlags_RoundCornersNone));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersTop", JS_NewInt32(ctx, ImDrawFlags_RoundCornersTop));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersBottom", JS_NewInt32(ctx, ImDrawFlags_RoundCornersBottom));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersLeft", JS_NewInt32(ctx, ImDrawFlags_RoundCornersLeft));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersRight", JS_NewInt32(ctx, ImDrawFlags_RoundCornersRight));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersAll", JS_NewInt32(ctx, ImDrawFlags_RoundCornersAll));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersDefault_", JS_NewInt32(ctx, ImDrawFlags_RoundCornersDefault_));
    JS_SetPropertyStr(ctx, imgui_obj, "ImDrawFlags_RoundCornersMask_", JS_NewInt32(ctx, ImDrawFlags_RoundCornersMask_));
}

ImDrawList* js_drawlist_type(uint32_t type) {
    switch (type) {
        case 0: return ImGui::GetForegroundDrawList();
        case 1: return ImGui::GetBackgroundDrawList();
        case 2: return ImGui::GetWindowDrawList();
    }
    return ImGui::GetWindowDrawList();
}

void js_imgui_init(JSContext* ctx) {
    const JSValue imgui_obj = JS_NewObject(ctx);
    const JSValue imgui_obj_enums = JS_NewObject(ctx);

    JS_SetPropertyStr(ctx, imgui_obj, "enums", imgui_obj_enums);
    js_imgui_enums(ctx, imgui_obj_enums);

    /*
     * Window Utilities
     */

    JS_CALL_BOOL("isWindowAppearing", ImGui::IsWindowAppearing());
    JS_CALL_BOOL("isWindowCollapsed", ImGui::IsWindowCollapsed());
    JS_FUNC("isWindowFocused", {
        JS_ENSURE_ARGS(1) JS_ARG(flags, uint32_t, Uint32, 0)
        return JS_NewBool(ctx, ImGui::IsWindowFocused(flags));
    })
    JS_FUNC("isWindowHovered", {
        JS_ENSURE_ARGS(1) JS_ARG(flags, uint32_t, Uint32, 0)
        return JS_NewBool(ctx, ImGui::IsWindowHovered(flags));
    })
    JS_FUNC("getWindowPos", {
        const auto pos = ImGui::GetWindowPos();
        return js_from_imvec2(ctx, pos);
    })
    JS_FUNC("getWindowSize", {
        const auto size = ImGui::GetWindowSize();
        return js_from_imvec2(ctx, size);
    })
    JS_FUNC("getWindowWidth", {
        return JS_NewFloat64(ctx, ImGui::GetWindowWidth());
    })
    JS_FUNC("getWindowHeight", {
        return JS_NewFloat64(ctx, ImGui::GetWindowHeight());
    })

    /*
     * Window Manipulation
     */

    JS_FUNC("setNextWindowPos", {
        JS_ENSURE_ARGS(3) JS_ARG_VEC2(pos, 0) JS_ARG(flags, uint32_t, Uint32, 1) JS_ARG_VEC2(pivot, 2)
        ImGui::SetNextWindowPos(pos, flags, pivot);
        return JS_UNDEFINED;
    })
    JS_FUNC("setNextWindowSize", {
        JS_ENSURE_ARGS(2) JS_ARG_VEC2(size, 0) JS_ARG(flags, uint32_t, Uint32, 1)
        ImGui::SetNextWindowSize(size, flags);
        return JS_UNDEFINED;
    })
    JS_FUNC("setNextWindowContentSize", {
        JS_ENSURE_ARGS(1) JS_ARG_VEC2(size, 0)
        ImGui::SetNextWindowContentSize(size);
        return JS_UNDEFINED;
    })
    JS_FUNC("setNextWindowCollapsed", {
        JS_ENSURE_ARGS(2) JS_ARG_BOOL(collapsed, 0) JS_ARG(flags, uint32_t, Uint32, 1)
        ImGui::SetNextWindowCollapsed(collapsed, flags);
        return JS_UNDEFINED;
    })
    JS_FUNC("setNextWindowFocus", {
        ImGui::SetNextWindowFocus();
        return JS_UNDEFINED;
    })
    JS_FUNC("setNextWindowScroll", {
        JS_ENSURE_ARGS(1) JS_ARG_VEC2(scroll, 0)
        ImGui::SetNextWindowScroll(scroll);
        return JS_UNDEFINED;
    })
    JS_FUNC("setNextWindowBgAlpha", {
        JS_ENSURE_ARGS(1) JS_ARG(alpha, double, Float64, 0)
        ImGui::SetNextWindowBgAlpha(alpha);
        return JS_UNDEFINED;
    })
    JS_FUNC("setWindowPos", {
        JS_ENSURE_ARGS(2) JS_ARG_VEC2(pos, 0) JS_ARG(flags, uint32_t, Uint32, 1)
        ImGui::SetWindowPos(pos, flags);
        return JS_UNDEFINED;
    })
    JS_FUNC("setWindowSize", {
        JS_ENSURE_ARGS(2) JS_ARG_VEC2(size, 0) JS_ARG(flags, uint32_t, Uint32, 1)
        ImGui::SetWindowSize(size, flags);
        return JS_UNDEFINED;
    })
    JS_FUNC("setWindowCollapsed", {
        JS_ENSURE_ARGS(2) JS_ARG_BOOL(collapsed, 0) JS_ARG(flags, uint32_t, Uint32, 1)
        ImGui::SetWindowCollapsed(collapsed, flags);
        return JS_UNDEFINED;
    })
    JS_FUNC("setWindowFocus", {
        ImGui::SetWindowFocus();
        return JS_UNDEFINED;
    })
    JS_FUNC("setWindowFontScale", {
        JS_ENSURE_ARGS(1) JS_ARG(scale, double, Float64, 0)
        ImGui::SetWindowFontScale(scale);
        return JS_UNDEFINED;
    })
    JS_FUNC("setWindowPosName", {
        JS_ENSURE_ARGS(3) JS_ARG_STRING(name, 0) JS_ARG_VEC2(pos, 1) JS_ARG(flags, uint32_t, Uint32, 2)
        ImGui::SetWindowPos(name, pos, flags);
        JS_FreeCString(ctx,name);
        return JS_UNDEFINED;
    })
    JS_FUNC("setWindowSizeName", {
        JS_ENSURE_ARGS(3) JS_ARG_STRING(name, 0) JS_ARG_VEC2(size, 1) JS_ARG(flags, uint32_t, Uint32, 2)
        ImGui::SetWindowSize(name, size, flags);
        JS_FreeCString(ctx,name);
        return JS_UNDEFINED;
    })
    JS_FUNC("setWindowCollapsedName", {
        JS_ENSURE_ARGS(3) JS_ARG_STRING(name, 0) JS_ARG_BOOL(collapsed, 1) JS_ARG(flags, uint32_t, Uint32, 2)
        ImGui::SetWindowCollapsed(name, collapsed, flags);
        JS_FreeCString(ctx,name);
        return JS_UNDEFINED;
    })
    JS_FUNC("setWindowFocusName", {
        JS_ENSURE_ARGS(1) JS_ARG_STRING(name, 0)
        ImGui::SetWindowFocus(name);
        JS_FreeCString(ctx,name);
        return JS_UNDEFINED;
    })

    /*
     * Content region
     */

    JS_FUNC("getContentRegionAvail", {
        const auto avail = ImGui::GetContentRegionAvail();
        return js_from_imvec2(ctx, avail);
    })
    JS_FUNC("getContentRegionMax", {
        const auto max = ImGui::GetContentRegionMax();
        return js_from_imvec2(ctx, max);
    })
    JS_FUNC("getWindowContentRegionMin", {
        const auto min = ImGui::GetWindowContentRegionMin();
        return js_from_imvec2(ctx, min);
    })
    JS_FUNC("getWindowContentRegionMax", {
        const auto max = ImGui::GetWindowContentRegionMax();
        return js_from_imvec2(ctx, max);
    })

    /*
     * Window Scrolling
     */

    JS_FUNC("getScrollX", {
        return JS_NewFloat64(ctx, ImGui::GetScrollX());
    })
    JS_FUNC("getScrollY", {
        return JS_NewFloat64(ctx, ImGui::GetScrollY());
    })
    JS_FUNC("setScrollX", {
        JS_ENSURE_ARGS(1) JS_ARG(scroll_x, double, Float64, 0)
        ImGui::SetScrollX(scroll_x);
        return JS_UNDEFINED;
    })
    JS_FUNC("setScrollY", {
        JS_ENSURE_ARGS(1) JS_ARG(scroll_y, double, Float64, 0)
        ImGui::SetScrollY(scroll_y);
        return JS_UNDEFINED;
    })
    JS_FUNC("getScrollMaxX", {
        return JS_NewFloat64(ctx, ImGui::GetScrollMaxX());
    })
    JS_FUNC("getScrollMaxY", {
        return JS_NewFloat64(ctx, ImGui::GetScrollMaxY());
    })
    JS_FUNC("setScrollHereX", {
        JS_ENSURE_ARGS(1) JS_ARG(center_x_ratio, double, Float64, 0)
        ImGui::SetScrollHereX(center_x_ratio);
        return JS_UNDEFINED;
    })
    JS_FUNC("setScrollHereY", {
        JS_ENSURE_ARGS(1) JS_ARG(center_y_ratio, double, Float64, 0)
        ImGui::SetScrollHereY(center_y_ratio);
        return JS_UNDEFINED;
    })
    JS_FUNC("setScrollFromPosX", {
        JS_ENSURE_ARGS(2) JS_ARG(local_x, double, Float64, 0) JS_ARG(center_x_ratio, double, Float64, 1)
        ImGui::SetScrollFromPosX(local_x, center_x_ratio);
        return JS_UNDEFINED;
    })
    JS_FUNC("setScrollFromPosY", {
        JS_ENSURE_ARGS(2) JS_ARG(local_y, double, Float64, 0) JS_ARG(center_y_ratio, double, Float64, 1)
        ImGui::SetScrollFromPosY(local_y, center_y_ratio);
        return JS_UNDEFINED;
    })

    /*
     * Parameters stacks (shared)
     */

    // TODO: pushFont

    JS_FUNC("popFont", {
        ImGui::PopFont();
        return JS_UNDEFINED;
    })
    JS_FUNC("pushStyleColor", {
        JS_ENSURE_ARGS(2) JS_ARG(color, uint32_t, Uint32, 0) JS_ARG(idx, uint32_t, Uint32, 1)
        ImGui::PushStyleColor(idx, color);
        return JS_UNDEFINED;
    })
    JS_FUNC("popStyleColor", {
        JS_ENSURE_ARGS(1) JS_ARG(count, int32_t, Int32, 0)
        ImGui::PopStyleColor(count);
        return JS_UNDEFINED;
    })
    JS_FUNC("pushStyleVarFloat", {
        JS_ENSURE_ARGS(2) JS_ARG(idx, uint32_t, Uint32, 0) JS_ARG(val, double, Float64, 1)
        ImGui::PushStyleVar(idx, val);
        return JS_UNDEFINED;
    })
    JS_FUNC("pushStyleVarVec2", {
        JS_ENSURE_ARGS(2) JS_ARG(idx, uint32_t, Uint32, 0) JS_ARG_VEC2(val, 1)
        ImGui::PushStyleVar(idx, val);
        return JS_UNDEFINED;
    })
    JS_FUNC("popStyleVar", {
        JS_ENSURE_ARGS(1) JS_ARG(count, int32_t, Int32, 0)
        ImGui::PopStyleVar(count);
        return JS_UNDEFINED;
    })
    JS_FUNC("pushTabStop", {
        JS_ENSURE_ARGS(1) JS_ARG_BOOL(tab_stop, 0)
        ImGui::PushTabStop(tab_stop);
        return JS_UNDEFINED;
    })
    JS_FUNC("popTabStop", {
        ImGui::PopTabStop();
        return JS_UNDEFINED;
    })
    JS_FUNC("pushButtonRepeat", {
        JS_ENSURE_ARGS(1) JS_ARG_BOOL(repeat, 0)
        ImGui::PushButtonRepeat(repeat);
        return JS_UNDEFINED;
    })
    JS_FUNC("popButtonRepeat", {
        ImGui::PopButtonRepeat();
        return JS_UNDEFINED;
    })

    /*
     * Parameters stacks (current window)
     */

    JS_FUNC("PushItemWidth", {
        JS_ENSURE_ARGS(1) JS_ARG(item_width, double, Float64, 0)
        ImGui::PushItemWidth(item_width);
        return JS_UNDEFINED;
    })
    JS_FUNC("PopItemWidth", {
        ImGui::PopItemWidth();
        return JS_UNDEFINED;
    })
    JS_FUNC("SetNextItemWidth", {
        JS_ENSURE_ARGS(1) JS_ARG(item_width, double, Float64, 0)
        ImGui::SetNextItemWidth(item_width);
        return JS_UNDEFINED;
    })
    JS_FUNC("CalcItemWidth", {
        return JS_NewFloat64(ctx, ImGui::CalcItemWidth());
    })
    JS_FUNC("PushTextWrapPos", {
        JS_ENSURE_ARGS(1) JS_ARG(wrap_local_pos_x, double, Float64, 0)
        ImGui::PushTextWrapPos(wrap_local_pos_x);
        return JS_UNDEFINED;
    })
    JS_FUNC("PopTextWrapPos", {
        ImGui::PopTextWrapPos();
        return JS_UNDEFINED;
    })

    /*
     * Style read access
     */

    // TODO: getFont

    JS_FUNC("getFontSize", {
        return JS_NewFloat64(ctx, ImGui::GetFontSize());
    })
    JS_FUNC("getFontTexUvWhitePixel", {
        const auto uv = ImGui::GetFontTexUvWhitePixel();
        return js_from_imvec2(ctx, uv);
    })
    JS_FUNC("getColorU32", {
        JS_ENSURE_ARGS(2) JS_ARG(idx, uint32_t, Uint32, 0) JS_ARG(alpha_mul, double, Float64, 1)
        const auto col = ImGui::GetColorU32(idx, alpha_mul);
        return JS_NewInt32(ctx, col);
    })
    JS_FUNC("getColorU32Vec4", {
        JS_ENSURE_ARGS(1) JS_ARG_VEC4(col, 0)
        const auto col_u32 = ImGui::GetColorU32(col);
        return JS_NewInt32(ctx, col_u32);
    })
    JS_FUNC("getColorU32U32", {
        JS_ENSURE_ARGS(1) JS_ARG(col, uint32_t, Uint32, 0)
        const auto col_u32 = ImGui::GetColorU32(col);
        return JS_NewInt32(ctx, col_u32);
    })
    JS_FUNC("getStyleColorVec4", {
        JS_ENSURE_ARGS(1) JS_ARG(idx, uint32_t, Uint32, 0)
        const auto col = ImGui::GetStyleColorVec4(idx);
        return js_from_imvec4(ctx, col);
    })

    /*
     * Layout cursor positioning
     */

    JS_FUNC("GetCursorScreenPos", {
        const auto pos = ImGui::GetCursorScreenPos();
        return js_from_imvec2(ctx, pos);
    })
    JS_FUNC("GetCursorPos", {
        const auto pos = ImGui::GetCursorPos();
        return js_from_imvec2(ctx, pos);
    })
    JS_FUNC("GetCursorPosX", {
        return JS_NewFloat64(ctx, ImGui::GetCursorPosX());
    })
    JS_FUNC("GetCursorPosY", {
        return JS_NewFloat64(ctx, ImGui::GetCursorPosY());
    })
    JS_FUNC("SetCursorPos", {
        JS_ENSURE_ARGS(1) JS_ARG_VEC2(local_pos, 0)
        ImGui::SetCursorPos(local_pos);
        return JS_UNDEFINED;
    })
    JS_FUNC("SetCursorPosX", {
        JS_ENSURE_ARGS(1) JS_ARG(local_x, double, Float64, 0)
        ImGui::SetCursorPosX(local_x);
        return JS_UNDEFINED;
    })
    JS_FUNC("SetCursorPosY", {
        JS_ENSURE_ARGS(1) JS_ARG(local_y, double, Float64, 0)
        ImGui::SetCursorPosY(local_y);
        return JS_UNDEFINED;
    })
    JS_FUNC("GetCursorStartPos", {
        const auto pos = ImGui::GetCursorStartPos();
        return js_from_imvec2(ctx, pos);
    })

    /*
     * Other layout functions
     */

    JS_CALL_VOID("separator", ImGui::Separator());
    JS_FUNC("sameLine", {
        JS_ENSURE_ARGS(2) JS_ARG(offset_from_start_x, double, Float64, 0) JS_ARG(spacing, double, Float64, 1)
        ImGui::SameLine(offset_from_start_x, spacing);
        return JS_UNDEFINED;
    })
    JS_CALL_VOID("newLine", ImGui::NewLine());
    JS_CALL_VOID("spacing", ImGui::Spacing());
    JS_FUNC("dummy", {
        JS_ENSURE_ARGS(1) JS_ARG_VEC2(size, 0)
        ImGui::Dummy(size);
        return JS_UNDEFINED;
    })
    JS_FUNC("indent", {
        JS_ENSURE_ARGS(1) JS_ARG(indent_w, double, Float64, 0)
        ImGui::Indent(indent_w);
        return JS_UNDEFINED;
    })
    JS_FUNC("unindent", {
        JS_ENSURE_ARGS(1) JS_ARG(indent_w, double, Float64, 0)
        ImGui::Unindent(indent_w);
        return JS_UNDEFINED;
    })
    JS_CALL_VOID("beginGroup", ImGui::BeginGroup());
    JS_CALL_VOID("endGroup", ImGui::EndGroup());
    JS_CALL_VOID("alignTextToFramePadding", ImGui::AlignTextToFramePadding());
    JS_FUNC("getTextLineHeight", {
        return JS_NewFloat64(ctx, ImGui::GetTextLineHeight());
    })
    JS_FUNC("getTextLineHeightWithSpacing", {
        return JS_NewFloat64(ctx, ImGui::GetTextLineHeightWithSpacing());
    })
    JS_FUNC("getFrameHeight", {
        return JS_NewFloat64(ctx, ImGui::GetFrameHeight());
    })
    JS_FUNC("getFrameHeightWithSpacing", {
        return JS_NewFloat64(ctx, ImGui::GetFrameHeightWithSpacing());
    })

    /*
     * ID stack/scopes
     */

    // TODO: add missing

    /*
     * Widgets: Text
     */

    JS_CALL_STRING("text", ImGui::Text("%s", arg))
    JS_FUNC("textColored", {
        JS_ENSURE_ARGS(2) JS_ARG_VEC4(col, 0) JS_ARG_STRING(text, 1)
        ImGui::TextColored(col, "%s", text);
        JS_FreeCString(ctx,text);
        return JS_UNDEFINED;
    })
    JS_FUNC("textDisabled", {
        JS_ENSURE_ARGS(1) JS_ARG_STRING(text, 0)
        ImGui::TextDisabled("%s", text);
        JS_FreeCString(ctx,text);
        return JS_UNDEFINED;
    })
    JS_FUNC("textWrapped", {
        JS_ENSURE_ARGS(1) JS_ARG_STRING(text, 0)
        ImGui::TextWrapped("%s", text);
        JS_FreeCString(ctx,text);
        return JS_UNDEFINED;
    })
    JS_FUNC("textUnformatted", {
        JS_ENSURE_ARGS(1) JS_ARG_STRING(text, 0)
        ImGui::TextUnformatted("%s", text);
        JS_FreeCString(ctx,text);
        return JS_UNDEFINED;
    })
    JS_FUNC("labelText", {
        JS_ENSURE_ARGS(2) JS_ARG_STRING(label, 0) JS_ARG_STRING(text, 1)
        ImGui::LabelText(label, "%s", text);
        JS_FreeCString(ctx,label);
        JS_FreeCString(ctx,text);
        return JS_UNDEFINED;
    })
    JS_FUNC("bulletText", {
        JS_ENSURE_ARGS(1) JS_ARG_STRING(text, 0)
        ImGui::BulletText("%s", text);
        JS_FreeCString(ctx,text);
        return JS_UNDEFINED;
    })
    JS_FUNC("separatorText", {
        JS_ENSURE_ARGS(1) JS_ARG_STRING(text, 0)
        ImGui::SeparatorText(text);
        JS_FreeCString(ctx,text);
        return JS_UNDEFINED;
    })

    // ========================================================================

    JS_FUNC("begin", {
        JS_ENSURE_ARGS(2) JS_ARG_STRING(name, 0) JS_ARG(flags, uint32_t, Uint32, 1)
        ImGui::Begin(name, nullptr, flags);
        JS_FreeCString(ctx,name);
        return JS_UNDEFINED;
    })

    JS_FUNC("beginMenu", {
        JS_ENSURE_ARGS(1) JS_ARG_STRING(name, 0) JS_ARG_BOOL(enabled, 1)
        ImGui::BeginMenu(name, enabled);
        JS_FreeCString(ctx,name);
        return JS_UNDEFINED;
    })

    JS_FUNC("button", {
        JS_ENSURE_ARGS(1) JS_ARG_STRING(label, 0)
        const auto pressed = ImGui::Button(label);
        JS_FreeCString(ctx,label);
        return JS_NewBool(ctx, pressed);
    })

    JS_FUNC("checkbox", {
        JS_ENSURE_ARGS(2) JS_ARG_STRING(label, 0) JS_ARG_BOOL(checked, 1)
        ImGui::Checkbox(label, &checked);
        JS_FreeCString(ctx,label);
        return JS_NewBool(ctx, checked);
    })

    JS_FUNC( "collapsingHeader", {
        JS_ENSURE_ARGS(2) JS_ARG_STRING(label, 0) JS_ARG(flags, uint32_t, Uint32, 1)
        bool open = ImGui::CollapsingHeader(label, flags);
        JS_FreeCString(ctx,label);
        return JS_NewBool(ctx, open);
    })

    /*
     * DrawList Functions
     */

    JS_FUNC("dlAddLine", {
        JS_ENSURE_ARGS(5) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p1, 1) JS_ARG_VEC2(p2, 2) JS_ARG(col, uint32_t, Uint32, 3) JS_ARG(thickness, double, Float64, 4)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddLine(p1, p2, col, thickness);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddRect", {
        JS_ENSURE_ARGS(7) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p_min, 1) JS_ARG_VEC2(p_max, 2) JS_ARG(col, uint32_t, Uint32, 3) JS_ARG(rounding, double, Float64, 4) JS_ARG(flags, uint32_t, Uint32, 5) JS_ARG(thickness, double, Float64, 6)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddRect(p_min, p_max, col, rounding, flags, thickness);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddRectFilled", {
        JS_ENSURE_ARGS(6) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p_min, 1) JS_ARG_VEC2(p_max, 2) JS_ARG(col, uint32_t, Uint32, 3) JS_ARG(rounding, double, Float64, 4) JS_ARG(flags, uint32_t, Uint32, 5)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddRectFilled(p_min, p_max, col, rounding, flags);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddRectFilledMultiColor", {
        JS_ENSURE_ARGS(6) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p_min, 1) JS_ARG_VEC2(p_max, 2) JS_ARG(col_upr_left, uint32_t, Uint32, 3) JS_ARG(col_upr_right, uint32_t, Uint32, 4) JS_ARG(col_bot_right, uint32_t, Uint32, 5) JS_ARG(col_bot_left, uint32_t, Uint32, 6)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddRectFilledMultiColor(p_min, p_max, col_upr_left, col_upr_right, col_bot_right, col_bot_left);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddQuad", {
        JS_ENSURE_ARGS(6) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p1, 1) JS_ARG_VEC2(p2, 2) JS_ARG_VEC2(p3, 3) JS_ARG_VEC2(p4, 4) JS_ARG(col, uint32_t, Uint32, 5) JS_ARG(thickness, double, Float64, 6)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddQuad(p1, p2, p3, p4, col, thickness);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddQuadFilled", {
        JS_ENSURE_ARGS(5) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p1, 1) JS_ARG_VEC2(p2, 2) JS_ARG_VEC2(p3, 3) JS_ARG_VEC2(p4, 4) JS_ARG(col, uint32_t, Uint32, 5)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddQuadFilled(p1, p2, p3, p4, col);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddTriangle", {
        JS_ENSURE_ARGS(5) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p1, 1) JS_ARG_VEC2(p2, 2) JS_ARG_VEC2(p3, 3) JS_ARG(col, uint32_t, Uint32, 4) JS_ARG(thickness, double, Float64, 5)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddTriangle(p1, p2, p3, col, thickness);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddTriangleFilled", {
        JS_ENSURE_ARGS(4) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p1, 1) JS_ARG_VEC2(p2, 2) JS_ARG_VEC2(p3, 3) JS_ARG(col, uint32_t, Uint32, 4)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddTriangleFilled(p1, p2, p3, col);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddCircle", {
        JS_ENSURE_ARGS(6) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(center, 1) JS_ARG(radius, double, Float64, 2) JS_ARG(col, uint32_t, Uint32, 3) JS_ARG(num_segments, int32_t, Int32, 4) JS_ARG(thickness, double, Float64, 5)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddCircle(center, radius, col, num_segments, thickness);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddCircleFilled", {
        JS_ENSURE_ARGS(5) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(center, 1) JS_ARG(radius, double, Float64, 2) JS_ARG(col, uint32_t, Uint32, 3) JS_ARG(num_segments, int32_t, Int32, 4)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddCircleFilled(center, radius, col, num_segments);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddNgon", {
        JS_ENSURE_ARGS(6) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(center, 1) JS_ARG(radius, double, Float64, 2) JS_ARG(col, uint32_t, Uint32, 3) JS_ARG(num_segments, int32_t, Int32, 4) JS_ARG(thickness, double, Float64, 5)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddNgon(center, radius, col, num_segments, thickness);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddNgonFilled", {
        JS_ENSURE_ARGS(5) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(center, 1) JS_ARG(radius, double, Float64, 2) JS_ARG(col, uint32_t, Uint32, 3) JS_ARG(num_segments, int32_t, Int32, 4)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddNgonFilled(center, radius, col, num_segments);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddEllipse", {
        JS_ENSURE_ARGS(8) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(center, 1) JS_ARG(radius_x, double, Float64, 2) JS_ARG(radius_y, double, Float64, 3) JS_ARG(col, uint32_t, Uint32, 4) JS_ARG(rot, double, Float64, 5) JS_ARG(num_segments, int32_t, Int32, 6) JS_ARG(thickness, double, Float64, 7)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddEllipse(center, radius_x, radius_y, col, rot, num_segments, thickness);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddEllipseFilled", {
        JS_ENSURE_ARGS(7) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(center, 1) JS_ARG(radius_x, double, Float64, 2) JS_ARG(radius_y, double, Float64, 3) JS_ARG(col, uint32_t, Uint32, 4) JS_ARG(rot, double, Float64, 5) JS_ARG(num_segments, int32_t, Int32, 6)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddEllipseFilled(center, radius_x, radius_y, col, rot, num_segments);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddText", {
        JS_ENSURE_ARGS(4) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(pos, 1) JS_ARG(col, uint32_t, Uint32, 2) JS_ARG_STRING(text, 3)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddText(pos, col, text);
        JS_FreeCString(ctx,text);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddBezierCubic", {
        JS_ENSURE_ARGS(7) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p1, 1) JS_ARG_VEC2(p2, 2) JS_ARG_VEC2(p3, 3) JS_ARG_VEC2(p4, 4) JS_ARG(col, uint32_t, Uint32, 5) JS_ARG(thickness, double, Float64, 6) JS_ARG(num_segments, int32_t, Int32, 7)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddBezierCubic(p1, p2, p3, p4, col, thickness, num_segments);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlAddBezierQuadratic", {
        JS_ENSURE_ARGS(6) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p1, 1) JS_ARG_VEC2(p2, 2) JS_ARG_VEC2(p3, 3) JS_ARG(col, uint32_t, Uint32, 4) JS_ARG(thickness, double, Float64, 5) JS_ARG(num_segments, int32_t, Int32, 6)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->AddBezierQuadratic(p1, p2, p3, col, thickness, num_segments);
        return JS_UNDEFINED;
    })

    // TODO: Add missing:
    // IMGUI_API void  AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const ImVec4* cpu_fine_clip_rect = NULL);
    // IMGUI_API void  AddPolyline(const ImVec2* points, int num_points, ImU32 col, ImDrawFlags flags, float thickness);
    // IMGUI_API void  AddConvexPolyFilled(const ImVec2* points, int num_points, ImU32 col);

    /*
     * Functions with no arguments
     */

    JS_CALL_VOID("beginGroup", ImGui::BeginGroup())
    JS_CALL_VOID("beginMainMenuBar", ImGui::BeginMainMenuBar())

    JS_CALL_VOID("end", ImGui::End())
    JS_CALL_VOID("endChild", ImGui::EndChild())
    JS_CALL_VOID("endGroup", ImGui::EndGroup())
    JS_CALL_VOID("endMainMenuBar", ImGui::EndMainMenuBar())
    JS_CALL_VOID("endMenu", ImGui::EndMenu())
    JS_CALL_VOID("endMenuBar", ImGui::EndMenuBar())
    JS_CALL_VOID("endPopup", ImGui::EndPopup())
    JS_CALL_VOID("endTabBar", ImGui::EndTabBar())
    JS_CALL_VOID("endTabItem", ImGui::EndTabItem())
    JS_CALL_VOID("endTooltip", ImGui::EndTooltip())

    JS_CALL_VOID("showDemoWindow", ImGui::ShowDemoWindow())
    JS_CALL_VOID("showMetricsWindow", ImGui::ShowMetricsWindow())
    JS_CALL_VOID("showUserGuide", ImGui::ShowUserGuide())
    JS_CALL_VOID("showStyleEditor", ImGui::ShowStyleEditor())

    JS_SetProperty(ctx, JS_GetGlobalObject(ctx), JS_NewAtom(ctx, "imgui"), imgui_obj);
}