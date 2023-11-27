#include "js_imgui.h"

#include <tuple>
#include <unordered_map>
#include <string>

#include "./imgui.h"
#include "./imgui_internal.h"

#define JS_CALL_VOID(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    FN; \
    return JS_UNDEFINED; \
}, NAME, strlen(NAME)));               \

#define JS_CALL_STRING(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    const auto arg = JS_ToCString(ctx, argv[0]); \
    FN; \
    JS_FreeCString(ctx, arg); \
    return JS_UNDEFINED; \
}, NAME, strlen(NAME))); \

#define JS_CALL_FLOAT(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    double arg = 0.0; \
    JS_ToFloat64(ctx, &arg, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, NAME, strlen(NAME))); \

#define JS_CALL_VEC2(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    const auto arg = js_to_imvec2(ctx, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, NAME, strlen(NAME))); \

#define JS_CALL_VEC4(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    const auto arg = js_to_imvec4(ctx, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, NAME, strlen(NAME))); \

#define JS_CALL_BOOL(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    bool arg = JS_ToBool(ctx, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, NAME, strlen(NAME))); \

#define JS_CALL_UINT32(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    uint32_t arg = 0; \
    JS_ToUint32(ctx, &arg, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, NAME, strlen(NAME))); \

#define JS_CALL_INT32(NAME, FN) \
JS_SetPropertyStr(ctx, imgui_obj, NAME, JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue { \
    if (argc < 1) {  return JS_EXCEPTION; } \
    int32_t arg = 0; \
    JS_ToInt32(ctx, &arg, argv[0]); \
    FN; \
    return JS_UNDEFINED; \
}, NAME, strlen(NAME))); \

#define JS_FUNC(NAME, FN_BODY) \
JS_SetProperty(ctx, imgui_obj, JS_NewAtom(ctx, NAME), JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) -> JSValue FN_BODY, NAME, strlen(NAME))); \

#define JS_ENSURE_ARGS(N) if (argc < N) { printf("imgui argument count mismatch! expected=%d got=%d\n", N, argc); JS_ThrowInternalError(ctx, "imgui argument count mismatch! expected=%d got=%d\n", N, argc); return JS_EXCEPTION; }
#define JS_ARG(NAME, TYPE, JS_TYPE, N) TYPE NAME = 0; JS_To##JS_TYPE(ctx, &NAME, argv[N]);
#define JS_ARG_BOOL(NAME, N) bool NAME = JS_ToBool(ctx, argv[N]);
#define JS_ARG_VEC2(NAME, N) const auto NAME = js_to_imvec2(ctx, argv[N]);
#define JS_ARG_VEC4(NAME, N) const auto NAME = js_to_imvec4(ctx, argv[N]);
#define JS_ARG_RECT(NAME, N) const auto NAME = js_to_imrect(ctx, argv[N]);
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

ImRect js_to_imrect(JSContext* ctx, JSValueConst val) {
    ImVec2 min = js_to_imvec2(ctx, JS_GetPropertyStr(ctx, val, "min"));
    ImVec2 max = js_to_imvec2(ctx, JS_GetPropertyStr(ctx, val, "max"));
    return ImRect(min, max);
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

JSValue js_from_imrect(JSContext* ctx, ImRect rect) {
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "min", js_from_imvec2(ctx, rect.Min));
    JS_SetPropertyStr(ctx, obj, "max", js_from_imvec2(ctx, rect.Max));
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

    // ImGuiChildFlags_
    JS_SetPropertyStr(ctx, imgui_obj, "ChildFlags_None", JS_NewInt32(ctx, ImGuiChildFlags_None));
    JS_SetPropertyStr(ctx, imgui_obj, "ChildFlags_Border", JS_NewInt32(ctx, ImGuiChildFlags_Border));
    JS_SetPropertyStr(ctx, imgui_obj, "ChildFlags_AlwaysUseWindowPadding", JS_NewInt32(ctx, ImGuiChildFlags_AlwaysUseWindowPadding));
    JS_SetPropertyStr(ctx, imgui_obj, "ChildFlags_ResizeX", JS_NewInt32(ctx, ImGuiChildFlags_ResizeX));
    JS_SetPropertyStr(ctx, imgui_obj, "ChildFlags_ResizeY", JS_NewInt32(ctx, ImGuiChildFlags_ResizeY));
    JS_SetPropertyStr(ctx, imgui_obj, "ChildFlags_AutoResizeX", JS_NewInt32(ctx, ImGuiChildFlags_AutoResizeX));
    JS_SetPropertyStr(ctx, imgui_obj, "ChildFlags_AutoResizeY", JS_NewInt32(ctx, ImGuiChildFlags_AutoResizeY));
    JS_SetPropertyStr(ctx, imgui_obj, "ChildFlags_AlwaysAutoResize", JS_NewInt32(ctx, ImGuiChildFlags_AlwaysAutoResize));
    JS_SetPropertyStr(ctx, imgui_obj, "ChildFlags_FrameStyle", JS_NewInt32(ctx, ImGuiChildFlags_FrameStyle));

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

    // ImGuiCol_
    JS_SetPropertyStr(ctx, imgui_obj, "Col_Text", JS_NewInt32(ctx, ImGuiCol_Text));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TextDisabled", JS_NewInt32(ctx, ImGuiCol_TextDisabled));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_WindowBg", JS_NewInt32(ctx, ImGuiCol_WindowBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_ChildBg", JS_NewInt32(ctx, ImGuiCol_ChildBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_PopupBg", JS_NewInt32(ctx, ImGuiCol_PopupBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_Border", JS_NewInt32(ctx, ImGuiCol_Border));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_BorderShadow", JS_NewInt32(ctx, ImGuiCol_BorderShadow));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_FrameBg", JS_NewInt32(ctx, ImGuiCol_FrameBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_FrameBgHovered", JS_NewInt32(ctx, ImGuiCol_FrameBgHovered));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_FrameBgActive", JS_NewInt32(ctx, ImGuiCol_FrameBgActive));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TitleBg", JS_NewInt32(ctx, ImGuiCol_TitleBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TitleBgActive", JS_NewInt32(ctx, ImGuiCol_TitleBgActive));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TitleBgCollapsed", JS_NewInt32(ctx, ImGuiCol_TitleBgCollapsed));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_MenuBarBg", JS_NewInt32(ctx, ImGuiCol_MenuBarBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_ScrollbarBg", JS_NewInt32(ctx, ImGuiCol_ScrollbarBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_ScrollbarGrab", JS_NewInt32(ctx, ImGuiCol_ScrollbarGrab));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_ScrollbarGrabHovered", JS_NewInt32(ctx, ImGuiCol_ScrollbarGrabHovered));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_ScrollbarGrabActive", JS_NewInt32(ctx, ImGuiCol_ScrollbarGrabActive));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_CheckMark", JS_NewInt32(ctx, ImGuiCol_CheckMark));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_SliderGrab", JS_NewInt32(ctx, ImGuiCol_SliderGrab));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_SliderGrabActive", JS_NewInt32(ctx, ImGuiCol_SliderGrabActive));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_Button", JS_NewInt32(ctx, ImGuiCol_Button));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_ButtonHovered", JS_NewInt32(ctx, ImGuiCol_ButtonHovered));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_ButtonActive", JS_NewInt32(ctx, ImGuiCol_ButtonActive));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_Header", JS_NewInt32(ctx, ImGuiCol_Header));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_HeaderHovered", JS_NewInt32(ctx, ImGuiCol_HeaderHovered));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_HeaderActive", JS_NewInt32(ctx, ImGuiCol_HeaderActive));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_Separator", JS_NewInt32(ctx, ImGuiCol_Separator));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_SeparatorHovered", JS_NewInt32(ctx, ImGuiCol_SeparatorHovered));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_SeparatorActive", JS_NewInt32(ctx, ImGuiCol_SeparatorActive));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_ResizeGrip", JS_NewInt32(ctx, ImGuiCol_ResizeGrip));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_ResizeGripHovered", JS_NewInt32(ctx, ImGuiCol_ResizeGripHovered));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_ResizeGripActive", JS_NewInt32(ctx, ImGuiCol_ResizeGripActive));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_Tab", JS_NewInt32(ctx, ImGuiCol_Tab));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TabHovered", JS_NewInt32(ctx, ImGuiCol_TabHovered));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TabActive", JS_NewInt32(ctx, ImGuiCol_TabActive));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TabUnfocused", JS_NewInt32(ctx, ImGuiCol_TabUnfocused));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TabUnfocusedActive", JS_NewInt32(ctx, ImGuiCol_TabUnfocusedActive));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_PlotLines", JS_NewInt32(ctx, ImGuiCol_PlotLines));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_PlotLinesHovered", JS_NewInt32(ctx, ImGuiCol_PlotLinesHovered));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_PlotHistogram", JS_NewInt32(ctx, ImGuiCol_PlotHistogram));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_PlotHistogramHovered", JS_NewInt32(ctx, ImGuiCol_PlotHistogramHovered));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TableHeaderBg", JS_NewInt32(ctx, ImGuiCol_TableHeaderBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TableBorderStrong", JS_NewInt32(ctx, ImGuiCol_TableBorderStrong));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TableBorderLight", JS_NewInt32(ctx, ImGuiCol_TableBorderLight));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TableRowBg", JS_NewInt32(ctx, ImGuiCol_TableRowBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TableRowBgAlt", JS_NewInt32(ctx, ImGuiCol_TableRowBgAlt));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_TextSelectedBg", JS_NewInt32(ctx, ImGuiCol_TextSelectedBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_DragDropTarget", JS_NewInt32(ctx, ImGuiCol_DragDropTarget));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_NavHighlight", JS_NewInt32(ctx, ImGuiCol_NavHighlight));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_NavWindowingHighlight", JS_NewInt32(ctx, ImGuiCol_NavWindowingHighlight));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_NavWindowingDimBg", JS_NewInt32(ctx, ImGuiCol_NavWindowingDimBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_ModalWindowDimBg", JS_NewInt32(ctx, ImGuiCol_ModalWindowDimBg));
    JS_SetPropertyStr(ctx, imgui_obj, "Col_COUNT", JS_NewInt32(ctx, ImGuiCol_COUNT));

    // ImGuiStyleVar_
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_Alpha", JS_NewInt32(ctx, ImGuiStyleVar_Alpha));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_DisabledAlpha", JS_NewInt32(ctx, ImGuiStyleVar_DisabledAlpha));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_WindowPadding", JS_NewInt32(ctx, ImGuiStyleVar_WindowPadding));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_WindowRounding", JS_NewInt32(ctx, ImGuiStyleVar_WindowRounding));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_WindowBorderSize", JS_NewInt32(ctx, ImGuiStyleVar_WindowBorderSize));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_WindowMinSize", JS_NewInt32(ctx, ImGuiStyleVar_WindowMinSize));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_WindowTitleAlign", JS_NewInt32(ctx, ImGuiStyleVar_WindowTitleAlign));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_ChildRounding", JS_NewInt32(ctx, ImGuiStyleVar_ChildRounding));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_ChildBorderSize", JS_NewInt32(ctx, ImGuiStyleVar_ChildBorderSize));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_PopupRounding", JS_NewInt32(ctx, ImGuiStyleVar_PopupRounding));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_PopupBorderSize", JS_NewInt32(ctx, ImGuiStyleVar_PopupBorderSize));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_FramePadding", JS_NewInt32(ctx, ImGuiStyleVar_FramePadding));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_FrameRounding", JS_NewInt32(ctx, ImGuiStyleVar_FrameRounding));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_FrameBorderSize", JS_NewInt32(ctx, ImGuiStyleVar_FrameBorderSize));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_ItemSpacing", JS_NewInt32(ctx, ImGuiStyleVar_ItemSpacing));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_ItemInnerSpacing", JS_NewInt32(ctx, ImGuiStyleVar_ItemInnerSpacing));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_IndentSpacing", JS_NewInt32(ctx, ImGuiStyleVar_IndentSpacing));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_CellPadding", JS_NewInt32(ctx, ImGuiStyleVar_CellPadding));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_ScrollbarSize", JS_NewInt32(ctx, ImGuiStyleVar_ScrollbarSize));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_ScrollbarRounding", JS_NewInt32(ctx, ImGuiStyleVar_ScrollbarRounding));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_GrabMinSize", JS_NewInt32(ctx, ImGuiStyleVar_GrabMinSize));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_GrabRounding", JS_NewInt32(ctx, ImGuiStyleVar_GrabRounding));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_TabRounding", JS_NewInt32(ctx, ImGuiStyleVar_TabRounding));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_TabBarBorderSize", JS_NewInt32(ctx, ImGuiStyleVar_TabBarBorderSize));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_ButtonTextAlign", JS_NewInt32(ctx, ImGuiStyleVar_ButtonTextAlign));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_SelectableTextAlign", JS_NewInt32(ctx, ImGuiStyleVar_SelectableTextAlign));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_SeparatorTextBorderSize", JS_NewInt32(ctx, ImGuiStyleVar_SeparatorTextBorderSize));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_SeparatorTextAlign", JS_NewInt32(ctx, ImGuiStyleVar_SeparatorTextAlign));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_SeparatorTextPadding", JS_NewInt32(ctx, ImGuiStyleVar_SeparatorTextPadding));
    JS_SetPropertyStr(ctx, imgui_obj, "StyleVar_COUNT", JS_NewInt32(ctx, ImGuiStyleVar_COUNT));

    // ImGuiMouseCursor_
    JS_SetPropertyStr(ctx, imgui_obj, "MouseCursor_None", JS_NewInt32(ctx, ImGuiMouseCursor_None));
    JS_SetPropertyStr(ctx, imgui_obj, "MouseCursor_Arrow", JS_NewInt32(ctx, ImGuiMouseCursor_Arrow));
    JS_SetPropertyStr(ctx, imgui_obj, "MouseCursor_TextInput", JS_NewInt32(ctx, ImGuiMouseCursor_TextInput));
    JS_SetPropertyStr(ctx, imgui_obj, "MouseCursor_ResizeAll", JS_NewInt32(ctx, ImGuiMouseCursor_ResizeAll));
    JS_SetPropertyStr(ctx, imgui_obj, "MouseCursor_ResizeNS", JS_NewInt32(ctx, ImGuiMouseCursor_ResizeNS));
    JS_SetPropertyStr(ctx, imgui_obj, "MouseCursor_ResizeEW", JS_NewInt32(ctx, ImGuiMouseCursor_ResizeEW));
    JS_SetPropertyStr(ctx, imgui_obj, "MouseCursor_ResizeNESW", JS_NewInt32(ctx, ImGuiMouseCursor_ResizeNESW));
    JS_SetPropertyStr(ctx, imgui_obj, "MouseCursor_ResizeNWSE", JS_NewInt32(ctx, ImGuiMouseCursor_ResizeNWSE));
    JS_SetPropertyStr(ctx, imgui_obj, "MouseCursor_Hand", JS_NewInt32(ctx, ImGuiMouseCursor_Hand));
    JS_SetPropertyStr(ctx, imgui_obj, "MouseCursor_NotAllowed", JS_NewInt32(ctx, ImGuiMouseCursor_NotAllowed));
    JS_SetPropertyStr(ctx, imgui_obj, "MouseCursor_COUNT", JS_NewInt32(ctx, ImGuiMouseCursor_COUNT));
}

ImDrawList* js_drawlist_type(uint32_t type) {
    switch (type) {
        case 0: return ImGui::GetForegroundDrawList();
        case 1: return ImGui::GetBackgroundDrawList();
        case 2: return ImGui::GetWindowDrawList();
    }
    return ImGui::GetWindowDrawList();
}

const ImWchar* get_glyph_ranges()
{
    static const ImWchar ranges[] =
        {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0xe000, 0xffff,
            0,
        };
    return &ranges[0];
}

static ImVec4 background_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static std::unordered_map<std::string, ImFont*> font_map;

void js_imgui_init(JSContext* ctx) {
    const JSValue imgui_obj = JS_NewObject(ctx);
    const JSValue imgui_obj_enums = JS_NewObject(ctx);

    JS_SetPropertyStr(ctx, imgui_obj, "enums", imgui_obj_enums);
    js_imgui_enums(ctx, imgui_obj_enums);

    /*
     * Additions
     */

    JS_FUNC("setBackgroundColor", {
        JS_ENSURE_ARGS(1) JS_ARG_VEC4(col, 0)
        background_color = col;
        return JS_UNDEFINED;
    })
    JS_FUNC("setBackgroundColorU32", {
        JS_ENSURE_ARGS(1) JS_ARG(col, uint32_t, Uint32, 0)
        background_color = ImGui::ColorConvertU32ToFloat4(col);
        return JS_UNDEFINED;
    })

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
    JS_FUNC("getDisplayWidth", {
        return JS_NewFloat64(ctx, ImGui::GetIO().DisplaySize.x);
    })
    JS_FUNC("getDisplayHeight", {
        return JS_NewFloat64(ctx, ImGui::GetIO().DisplaySize.y);
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
    JS_FUNC("getWindowContentRegionWidth", {
        // Deprecated in ImGui but useful to have
        return JS_NewFloat64(ctx, ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x);
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
        JS_ENSURE_ARGS(2) JS_ARG(idx, uint32_t, Uint32, 0) JS_ARG(color, uint32_t, Uint32, 1)
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

    JS_FUNC("pushItemWidth", {
        JS_ENSURE_ARGS(1) JS_ARG(item_width, double, Float64, 0)
        ImGui::PushItemWidth(item_width);
        return JS_UNDEFINED;
    })
    JS_FUNC("popItemWidth", {
        ImGui::PopItemWidth();
        return JS_UNDEFINED;
    })
    JS_FUNC("etNextItemWidth", {
        JS_ENSURE_ARGS(1) JS_ARG(item_width, double, Float64, 0)
        ImGui::SetNextItemWidth(item_width);
        return JS_UNDEFINED;
    })
    JS_FUNC("calcItemWidth", {
        return JS_NewFloat64(ctx, ImGui::CalcItemWidth());
    })
    JS_FUNC("pushTextWrapPos", {
        JS_ENSURE_ARGS(1) JS_ARG(wrap_local_pos_x, double, Float64, 0)
        ImGui::PushTextWrapPos(wrap_local_pos_x);
        return JS_UNDEFINED;
    })
    JS_FUNC("popTextWrapPos", {
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

    JS_FUNC("getCursorScreenPos", {
        const auto pos = ImGui::GetCursorScreenPos();
        return js_from_imvec2(ctx, pos);
    })
    JS_FUNC("getCursorPos", {
        const auto pos = ImGui::GetCursorPos();
        return js_from_imvec2(ctx, pos);
    })
    JS_FUNC("getCursorPosX", {
        return JS_NewFloat64(ctx, ImGui::GetCursorPosX());
    })
    JS_FUNC("getCursorPosY", {
        return JS_NewFloat64(ctx, ImGui::GetCursorPosY());
    })
    JS_FUNC("setCursorPos", {
        JS_ENSURE_ARGS(1) JS_ARG_VEC2(local_pos, 0)
        ImGui::SetCursorPos(local_pos);
        return JS_UNDEFINED;
    })
    JS_FUNC("setCursorPosX", {
        JS_ENSURE_ARGS(1) JS_ARG(local_x, double, Float64, 0)
        ImGui::SetCursorPosX(local_x);
        return JS_UNDEFINED;
    })
    JS_FUNC("setCursorPosY", {
        JS_ENSURE_ARGS(1) JS_ARG(local_y, double, Float64, 0)
        ImGui::SetCursorPosY(local_y);
        return JS_UNDEFINED;
    })
    JS_FUNC("getCursorStartPos", {
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

    /*
     * Text Utilities
     */

    JS_FUNC("calcTextSize", {
        JS_ENSURE_ARGS(4) JS_ARG_STRING(text, 0) JS_ARG_BOOL(hide_text_after_double_hash, 1) JS_ARG(wrap_width, double, Float64, 2)
        const auto size = ImGui::CalcTextSize(text, nullptr, hide_text_after_double_hash, wrap_width);
        JS_FreeCString(ctx,text);
        return js_from_imvec2(ctx, size);
    })
    JS_FUNC("calcTextSizeSimple", {
        JS_ENSURE_ARGS(1) JS_ARG_STRING(text, 0)
        const auto size = ImGui::CalcTextSize(text);
        JS_FreeCString(ctx,text);
        return js_from_imvec2(ctx, size);
    })

    /*
     * Color Utilities
     */

    JS_FUNC("colorConvertU32ToFloat4", {
        JS_ENSURE_ARGS(1) JS_ARG(col, uint32_t, Uint32, 0)
        const auto col_vec4 = ImGui::ColorConvertU32ToFloat4(col);
        return js_from_imvec4(ctx, col_vec4);
    })
    JS_FUNC("colorConvertFloat4ToU32", {
        JS_ENSURE_ARGS(1) JS_ARG_VEC4(col, 0)
        const auto col_u32 = ImGui::ColorConvertFloat4ToU32(col);
        return JS_NewInt32(ctx, col_u32);
    })

    /*
     * Basic Helpers for widget code
     */

    JS_FUNC("itemSize", {
        JS_ENSURE_ARGS(2) JS_ARG_VEC2(size, 0) JS_ARG(text_baseline_y, double, Float64, 1)
        ImGui::ItemSize(size, text_baseline_y);
        return JS_UNDEFINED;
    })
    JS_FUNC("itemAdd", {
        JS_ENSURE_ARGS(4) JS_ARG_VEC4(bb, 0) JS_ARG(id, uint32_t, Uint32, 1) JS_ARG_RECT(nav_bb, 2) JS_ARG(extra_flags, uint32_t, Uint32, 3)
        ImGui::ItemAdd(bb, id, &nav_bb, extra_flags);
        return JS_UNDEFINED;
    })
    JS_FUNC("itemHoverable", {
        JS_ENSURE_ARGS(3) JS_ARG_VEC4(bb, 0) JS_ARG(id, uint32_t, Uint32, 1) JS_ARG(item_flags, uint32_t, Uint32, 2)
        return JS_NewBool(ctx, ImGui::ItemHoverable(bb, id, item_flags));
    })
    // TODO: isWindowContentHoverable
    JS_FUNC("isClippedEx", {
        JS_ENSURE_ARGS(2) JS_ARG_VEC4(bb, 0) JS_ARG(id, uint32_t, Uint32, 1)
        return JS_NewBool(ctx, ImGui::IsClippedEx(bb, id));
    })
    // TODO: SetLastItemData
    JS_FUNC("calcItemSize", {
        JS_ENSURE_ARGS(3) JS_ARG_VEC2(size, 0) JS_ARG(default_w, double, Float64, 1) JS_ARG(default_h, double, Float64, 2)
        const auto calc_size = ImGui::CalcItemSize(size, default_w, default_h);
        return js_from_imvec2(ctx, calc_size);
    })
    // TODO: calcWrapWidthForPos
    JS_FUNC("pushMultiItemsWidths", {
        JS_ENSURE_ARGS(2) JS_ARG(components, int32_t, Int32, 0) JS_ARG(width_full, double, Float64, 1)
        ImGui::PushMultiItemsWidths(components, width_full);
        return JS_UNDEFINED;
    })
    JS_FUNC("isItemToggledSelection", {
        return JS_NewBool(ctx, ImGui::IsItemToggledSelection());
    })
    JS_FUNC("getContentRegionMaxAbs", {
        const auto max_abs = ImGui::GetContentRegionMaxAbs();
        return js_from_imvec2(ctx, max_abs);
    })

    /*
     * Mouse Utilities
     */

    JS_FUNC("isMouseDown", {
        JS_ENSURE_ARGS(1) JS_ARG(button, uint32_t, Uint32, 0)
        return JS_NewBool(ctx, ImGui::IsMouseDown(button));
    })
    JS_FUNC("isMouseClicked", {
        JS_ENSURE_ARGS(2) JS_ARG(button, uint32_t, Uint32, 0) JS_ARG_BOOL(repeat, 1)
        return JS_NewBool(ctx, ImGui::IsMouseClicked(button, repeat));
    })
    JS_FUNC("isMouseReleased", {
        JS_ENSURE_ARGS(1) JS_ARG(button, uint32_t, Uint32, 0)
        return JS_NewBool(ctx, ImGui::IsMouseReleased(button));
    })
    JS_FUNC("isMouseDoubleClicked", {
        JS_ENSURE_ARGS(1) JS_ARG(button, uint32_t, Uint32, 0)
        return JS_NewBool(ctx, ImGui::IsMouseDoubleClicked(button));
    })
    JS_FUNC("getMouseClickedCount", {
        JS_ENSURE_ARGS(1) JS_ARG(button, uint32_t, Uint32, 0)
        return JS_NewInt32(ctx, ImGui::GetMouseClickedCount(button));
    })
    JS_FUNC("isMouseHoveringRect", {
        JS_ENSURE_ARGS(3) JS_ARG_VEC2(r_min, 0) JS_ARG_VEC2(r_max, 1) JS_ARG_BOOL(clip, 2)
        return JS_NewBool(ctx, ImGui::IsMouseHoveringRect(r_min, r_max, clip));
    })
    JS_FUNC("isMouseHoveringRectWindow", {
        JS_ENSURE_ARGS(3) JS_ARG_VEC2(r_min, 0) JS_ARG_VEC2(r_max, 1) JS_ARG_BOOL(clip, 2)
        if (ImGui::GetCurrentContext()->HoveredWindow && (ImGui::GetCurrentContext()->HoveredWindow->ID != ImGui::GetCurrentWindow()->ID))
            return JS_NewBool(ctx, false);
        return JS_NewBool(ctx, ImGui::IsMouseHoveringRect(r_min, r_max, clip));
    })
    JS_FUNC("isMousePosValid", {
        return JS_NewBool(ctx, ImGui::IsMousePosValid());
    })
    JS_FUNC("isAnyMouseDown", {
        return JS_NewBool(ctx, ImGui::IsAnyMouseDown());
    })
    JS_FUNC("getMousePos", {
        const auto pos = ImGui::GetMousePos();
        return js_from_imvec2(ctx, pos);
    })
    JS_FUNC("getMousePosOnOpeningCurrentPopup", {
        const auto pos = ImGui::GetMousePosOnOpeningCurrentPopup();
        return js_from_imvec2(ctx, pos);
    })
    JS_FUNC("isMouseDragging", {
        JS_ENSURE_ARGS(2) JS_ARG(button, uint32_t, Uint32, 0) JS_ARG(lock_threshold, double, Float64, 1)
        return JS_NewBool(ctx, ImGui::IsMouseDragging(button, lock_threshold));
    })
    JS_FUNC("getMouseDragDelta", {
        JS_ENSURE_ARGS(2) JS_ARG(button, uint32_t, Uint32, 0) JS_ARG(lock_threshold, double, Float64, 1)
        const auto delta = ImGui::GetMouseDragDelta(button, lock_threshold);
        return js_from_imvec2(ctx, delta);
    })
    JS_FUNC("resetMouseDragDelta", {
        JS_ENSURE_ARGS(1) JS_ARG(button, uint32_t, Uint32, 0)
        ImGui::ResetMouseDragDelta(button);
        return JS_UNDEFINED;
    })
    JS_FUNC("getMouseCursor", {
        return JS_NewInt32(ctx, ImGui::GetMouseCursor());
    })
    JS_FUNC("setMouseCursor", {
        JS_ENSURE_ARGS(1) JS_ARG(cursor_type, uint32_t, Uint32, 0)
        ImGui::SetMouseCursor(cursor_type);
        return JS_UNDEFINED;
    })
    JS_FUNC("setNextFrameWantCaptureMouse", {
        JS_ENSURE_ARGS(1) JS_ARG_BOOL(want_capture_mouse, 0)
        ImGui::SetNextFrameWantCaptureMouse(want_capture_mouse);
        return JS_UNDEFINED;
    })

    // ========================================================================

    JS_FUNC("begin", {
        JS_ENSURE_ARGS(2) JS_ARG_STRING(name, 0) JS_ARG(flags, uint32_t, Uint32, 1)
        ImGui::Begin(name, nullptr, flags);
        JS_FreeCString(ctx,name);
        return JS_UNDEFINED;
    })
    JS_FUNC("beginChild", {
        JS_ENSURE_ARGS(4) JS_ARG_STRING(str_id, 0) JS_ARG_VEC2(size, 1) JS_ARG(child_flags, uint32_t, Uint32, 2) JS_ARG(window_flags, uint32_t, Uint32, 3)
        ImGui::BeginChild(str_id, size, child_flags, window_flags);
        JS_FreeCString(ctx,str_id);
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

    /*
     * inline    void  PathClear()                                                 { _Path.Size = 0; }
    inline    void  PathLineTo(const ImVec2& pos)                               { _Path.push_back(pos); }
    inline    void  PathLineToMergeDuplicate(const ImVec2& pos)                 { if (_Path.Size == 0 || memcmp(&_Path.Data[_Path.Size - 1], &pos, 8) != 0) _Path.push_back(pos); }
    inline    void  PathFillConvex(ImU32 col)                                   { AddConvexPolyFilled(_Path.Data, _Path.Size, col); _Path.Size = 0; }
    inline    void  PathStroke(ImU32 col, ImDrawFlags flags = 0, float thickness = 1.0f) { AddPolyline(_Path.Data, _Path.Size, col, flags, thickness); _Path.Size = 0; }
    IMGUI_API void  PathArcTo(const ImVec2& center, float radius, float a_min, float a_max, int num_segments = 0);
    IMGUI_API void  PathArcToFast(const ImVec2& center, float radius, int a_min_of_12, int a_max_of_12);                // Use precomputed angles for a 12 steps circle
    IMGUI_API void  PathEllipticalArcTo(const ImVec2& center, float radius_x, float radius_y, float rot, float a_min, float a_max, int num_segments = 0); // Ellipse
    IMGUI_API void  PathBezierCubicCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments = 0); // Cubic Bezier (4 control points)
    IMGUI_API void  PathBezierQuadraticCurveTo(const ImVec2& p2, const ImVec2& p3, int num_segments = 0);               // Quadratic Bezier (3 control points)
    IMGUI_API void  PathRect(const ImVec2& rect_min, const ImVec2& rect_max, float rounding = 0.0f, ImDrawFlags flags = 0);

     */

    JS_FUNC("dlPathClear", {
        JS_ENSURE_ARGS(1) JS_ARG(draw_list_type, uint32_t, Uint32, 0)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->PathClear();
        return JS_UNDEFINED;
    })
    JS_FUNC("dlPathLineTo", {
        JS_ENSURE_ARGS(2) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(pos, 1)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->PathLineTo(pos);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlPathLineToMergeDuplicate", {
        JS_ENSURE_ARGS(2) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(pos, 1)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->PathLineToMergeDuplicate(pos);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlPathFillConvex", {
        JS_ENSURE_ARGS(2) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG(col, uint32_t, Uint32, 1)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->PathFillConvex(col);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlPathStroke", {
        JS_ENSURE_ARGS(4) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG(col, uint32_t, Uint32, 1) JS_ARG(flags, uint32_t, Uint32, 2) JS_ARG(thickness, double, Float64, 3)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->PathStroke(col, flags, thickness);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlPathArcTo", {
        JS_ENSURE_ARGS(6) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(center, 1) JS_ARG(radius, double, Float64, 2) JS_ARG(a_min, double, Float64, 3) JS_ARG(a_max, double, Float64, 4) JS_ARG(num_segments, int32_t, Int32, 5)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->PathArcTo(center, radius, a_min, a_max, num_segments);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlPathArcToFast", {
        JS_ENSURE_ARGS(5) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(center, 1) JS_ARG(radius, double, Float64, 2) JS_ARG(a_min_of_12, int32_t, Int32, 3) JS_ARG(a_max_of_12, int32_t, Int32, 4)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->PathArcToFast(center, radius, a_min_of_12, a_max_of_12);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlPathEllipticalArcTo", {
        JS_ENSURE_ARGS(8) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(center, 1) JS_ARG(radius_x, double, Float64, 2) JS_ARG(radius_y, double, Float64, 3) JS_ARG(rot, double, Float64, 4) JS_ARG(a_min, double, Float64, 5) JS_ARG(a_max, double, Float64, 6) JS_ARG(num_segments, int32_t, Int32, 7)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->PathEllipticalArcTo(center, radius_x, radius_y, rot, a_min, a_max, num_segments);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlPathBezierCubicCurveTo", {
        JS_ENSURE_ARGS(5) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p2, 1) JS_ARG_VEC2(p3, 2) JS_ARG_VEC2(p4, 3) JS_ARG(num_segments, int32_t, Int32, 4)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->PathBezierCubicCurveTo(p2, p3, p4, num_segments);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlPathBezierQuadraticCurveTo", {
        JS_ENSURE_ARGS(4) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(p2, 1) JS_ARG_VEC2(p3, 2) JS_ARG(num_segments, int32_t, Int32, 3)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->PathBezierQuadraticCurveTo(p2, p3, num_segments);
        return JS_UNDEFINED;
    })
    JS_FUNC("dlPathRect", {
        JS_ENSURE_ARGS(5) JS_ARG(draw_list_type, uint32_t, Uint32, 0) JS_ARG_VEC2(rect_min, 1) JS_ARG_VEC2(rect_max, 2) JS_ARG(rounding, double, Float64, 3) JS_ARG(flags, uint32_t, Uint32, 4)
        auto draw_list = js_drawlist_type(draw_list_type);
        draw_list->PathRect(rect_min, rect_max, rounding, flags);
        return JS_UNDEFINED;
    })

    // TODO: Add missing:
    // IMGUI_API void  AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const ImVec4* cpu_fine_clip_rect = NULL);
    // IMGUI_API void  AddPolyline(const ImVec2* points, int num_points, ImU32 col, ImDrawFlags flags, float thickness);
    // IMGUI_API void  AddConvexPolyFilled(const ImVec2* points, int num_points, ImU32 col);

    /*
     * Font handling
     */

    JS_FUNC("addFontFromFileTTF", {
        JS_ENSURE_ARGS(3) JS_ARG_STRING(name, 0) JS_ARG_STRING(filename, 1) JS_ARG(font_size, double, Float64, 2)

        ImFontConfig conf;
        if (strcmp(getenv("CHIBI_HIGH_DPI"), "1") == 0) {
            conf.RasterizerDensity = 2.0f;
            conf.GlyphRanges = get_glyph_ranges();
        }

        const auto font = ImGui::GetIO().Fonts->AddFontFromFileTTF(filename, font_size, &conf);
        if (font == nullptr) {
            JS_FreeCString(ctx, name);
            JS_FreeCString(ctx,filename);
            return JS_UNDEFINED;
        }
        font_map[name] = font;
        JS_FreeCString(ctx, name);
        JS_FreeCString(ctx,filename);
        return JS_TRUE;
    })
    JS_FUNC("addFontFromFileTTFMerged", {
        JS_ENSURE_ARGS(7) JS_ARG_STRING(name, 0) JS_ARG_STRING(filename, 1) JS_ARG(font_size, double, Float64, 2) JS_ARG_STRING(icons, 3) JS_ARG(font_size_icons, double, Float64, 4) JS_ARG(range_a, uint32_t, Uint32, 5) JS_ARG(range_b, uint32_t, Uint32, 6)

        ImFontConfig conf;
        if (strcmp(getenv("CHIBI_HIGH_DPI"), "1") == 0) {
            conf.RasterizerDensity = 2.0f;
        }

        ImWchar* icon_ranges = (ImWchar*)malloc(sizeof(ImWchar) * 3);
        icon_ranges[0] = range_a;
        icon_ranges[1] = range_b;
        icon_ranges[2] = 0;

        ImFontConfig icons_merge;
        icons_merge.MergeMode = true;
        if (strcmp(getenv("CHIBI_HIGH_DPI"), "1") == 0) {
            icons_merge.RasterizerDensity = 2.0f;
        }

        // TODO: parse from JSValue

        const auto font = ImGui::GetIO().Fonts->AddFontFromFileTTF(filename, font_size, &conf);
        if (font == nullptr) {
            JS_FreeCString(ctx,filename);
            JS_FreeCString(ctx,name);
            JS_FreeCString(ctx, icons);
            return JS_UNDEFINED;
        }
        font_map[name] = font;

        ImGui::GetIO().Fonts->AddFontFromFileTTF(icons, font_size_icons, &icons_merge, icon_ranges);

        JS_FreeCString(ctx,filename);
        JS_FreeCString(ctx,name);
        JS_FreeCString(ctx, icons);
        return JS_TRUE;
    })
    JS_FUNC("pushFont", {
        JS_ENSURE_ARGS(1) JS_ARG_STRING(name, 0)
        const auto font = font_map[name];
        if (font == nullptr) {
            JS_FreeCString(ctx,name);
            return JS_UNDEFINED;
        }
        ImGui::PushFont(font);
        JS_FreeCString(ctx,name);
        return JS_TRUE;
    })
    JS_FUNC("setCurrentFont", {
        JS_ENSURE_ARGS(1) JS_ARG_STRING(name, 0)
        const auto font = font_map[name];
        if (font == nullptr) {
            JS_FreeCString(ctx,name);
            return JS_UNDEFINED;
        }
        ImGui::GetIO().FontDefault = font;
        JS_FreeCString(ctx,name);
        return JS_TRUE;
    })
    JS_CALL_VOID("popFont", ImGui::PopFont());

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

ImVec4* get_background_color() {
    return &background_color;
}