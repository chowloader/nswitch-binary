#ifndef __CHOWLOADER_ASSETS_H
#define __CHOWLOADER_ASSETS_H

#include "../lib/imports.h"
#include "../utils.h"

JSValue createAssetsObject(JSContext *ctx);
JSValue loadImage(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue isImageLoaded(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue isImagePreloaded(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue loadAudio(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue isAudioLoaded(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue isAudioPreloaded(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
std_string *hookFonts(std_string *path, const char *ext);
void hookBorder(std_string *path, void *imageid, void *width, void *height);

#endif