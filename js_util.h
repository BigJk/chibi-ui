#pragma once

#include <quickjs.h>

void dump_obj(JSContext *ctx, FILE *f, JSValueConst val);
void dump_error1(JSContext *ctx, JSValueConst exception_val);
void dump_error(JSContext *ctx);