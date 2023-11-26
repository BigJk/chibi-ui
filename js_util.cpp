#include "js_util.h"

void dump_obj(JSContext *ctx, FILE *f, JSValueConst val) {
    const char *str = JS_ToCString(ctx, val);
    if (str) {
        fprintf(f, "%s\n", str);
        JS_FreeCString(ctx, str);
    } else {
        fprintf(f, "[exception]\n");
    }
}

void dump_error1(JSContext *ctx, JSValueConst exception_val) {
    int is_error = JS_IsError(ctx, exception_val);
    dump_obj(ctx, stderr, exception_val);
    if (is_error) {
        JSValue val = JS_GetPropertyStr(ctx, exception_val, "stack");
        if (!JS_IsUndefined(val))
            dump_obj(ctx, stderr, val);
        JS_FreeValue(ctx, val);
    }
    fflush(stderr);
}

void dump_error(JSContext *ctx) {
    JSValue exception_val = JS_GetException(ctx);
    dump_error1(ctx, exception_val);
    JS_FreeValue(ctx, exception_val);
}