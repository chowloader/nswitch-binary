#ifndef __CHOWLOADER_ERROR_H
#define __CHOWLOADER_ERROR_H

#include "../lib/imports.h"
#include "../utils.h"

void hookBuildBacktrace(JSContext *ctx, JSValueConst error_obj, const char *filename, int line_num, int backtrace_flags);
JSValue hookThrow(JSContext *ctx, JSValue obj);

#endif