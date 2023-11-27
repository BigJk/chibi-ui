#pragma once

#include <quickjs.h>
#include <imgui.h>

void js_imgui_init(JSContext* ctx);
ImVec4* get_background_color();
