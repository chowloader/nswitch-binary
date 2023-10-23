#include "renderer.h"

JSValue createRendererObject(JSContext *ctx){
  JSValue renderer = JS_NewObject(ctx);

  JSValue _clear = JS_NewCFunction2(ctx, &clear, "clear", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, renderer, "clear", _clear);

  JSValue _draw = JS_NewCFunction2(ctx, &draw, "draw", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, renderer, "draw", _draw);

  JSValue _getCanvasDimension = JS_NewCFunction2(ctx, &getCanvasDimension, "getCanvasDimension", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, renderer, "getCanvasDimension", _getCanvasDimension);

  JSValue _getImageDimension = JS_NewCFunction2(ctx, &getImageDimension, "getImageDimension", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, renderer, "getImageDimension", _getImageDimension);

  return renderer;
}

char isDrawing = 0;
uint64_t Render_offset = 0;
uint64_t Render_offsetf = 0;

JSValue clear(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  if(isDrawing != 1){
    isDrawing = 1;
    platform_begin_draw();
  }
  uint32_t color = 0xFF000000;
  if(JS_VALUE_GET_TAG(argv[0]) == JS_TAG_STRING){
    const char* str_color = JS_ToCStringLen2(ctx, NULL, argv[0], 0);
    if(str_color){
      color = FontUtils_parse_color(str_color);
      JS_FreeCString(ctx, str_color);
    }
  }
  Render_offset = 0;
  Render_offsetf = 0;
  Render_set_view(0, 0, 640, 480);
  Render_clear(color);
  return JS_UNDEFINED;
}

JSValue draw(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  if(isDrawing != 1){
    isDrawing = 1;
    platform_begin_draw();
  }
  ImageUtils_update();
  platform_swap_buffers();
  isDrawing = 0;
  return JS_UNDEFINED;
}

uint32_t CanvasClassID = 1;

JSValue getCanvasDimension(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  ChowdrenCanvas* canvas = JS_GetOpaque(argv[0], CanvasClassID);
  JSValue dimensions = JS_NewArray(ctx);
  JS_DefinePropertyValueUint32(ctx, dimensions, 0, JS_MKVAL(JS_TAG_INT, canvas->width), 7);
  JS_DefinePropertyValueUint32(ctx, dimensions, 1, JS_MKVAL(JS_TAG_INT, canvas->height), 7);
  return dimensions;
}

JSValue getImageDimension(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  int imageid;
  JS_ToInt32(ctx, &imageid, argv[0]);
  ChowdrenCachedImage *image = get_cached_image(imageid);
  JSValue dimensions = JS_NewArray(ctx);
  JS_DefinePropertyValueUint32(ctx, dimensions, 0, JS_MKVAL(JS_TAG_INT, image->width), 7);
  JS_DefinePropertyValueUint32(ctx, dimensions, 1, JS_MKVAL(JS_TAG_INT, image->height), 7);
  return dimensions;
}