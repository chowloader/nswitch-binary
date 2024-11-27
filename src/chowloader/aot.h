#ifndef __CHOWLOADER_AOT_H
#define __CHOWLOADER_AOT_H

#include "../lib/imports.h"
#include "../utils.h"

JSValue getAOTObject(JSContext *ctx, const char* type, int count, void *va_list);
JSValue hookJSAOT(JSContext *ctx, int count, void *va_list);
JSValue hookJSVAL(JSContext *ctx, int count, void *va_list);
JSValue hookJSVARREF(JSContext *ctx, int count, void *va_list);
void initAOT(JSContext *ctx);

extern volatile JSValue JSVALOffset;

JSValue createAOTObject(JSContext *ctx);
JSValue findJSVALNative(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue patchJSVALNative(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

#endif