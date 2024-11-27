#include "error.h"

void hookBuildBacktrace(JSContext *ctx, JSValueConst error_obj, const char *filename, int line_num, int backtrace_flags){
  build_backtrace(ctx, error_obj, filename, line_num, backtrace_flags);
  emitChowloaderEventValue(ctx, "error", error_obj);
}

JSValue hookThrow(JSContext *ctx, JSValue obj){
  emitChowloaderEventValue(ctx, "error", obj);
  return JS_Throw(ctx, obj);
}