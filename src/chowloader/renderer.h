#ifndef __CHOWLOADER_RENDERER_H
#define __CHOWLOADER_RENDERER_H

#include "../lib/imports.h"

extern volatile char isDrawing;
extern volatile uint64_t Render_offset;
extern volatile uint64_t Render_offsetf;
extern volatile uint32_t CanvasClassID;

JSValue createRendererObject(JSContext *ctx);
JSValue clear(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue draw(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue getCanvasDimension(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue getImageDimension(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

#endif