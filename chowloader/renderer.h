#ifndef __CHOWLOADER_RENDERER_H
#define __CHOWLOADER_RENDERER_H

JSValue createRendererObject(JSContext *ctx);
JSValue clear(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue draw(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue getCanvasDimension(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue getImageDimension(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

#endif