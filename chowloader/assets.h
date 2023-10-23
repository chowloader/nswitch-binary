#ifndef __CHOWLOADER_ASSETS_H
#define __CHOWLOADER_ASSETS_H

#include "../lib/imports.h"
#include "../utils.h"

JSValue createAssetsObject(JSContext *ctx);
JSValue loadImage(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue loadAudio(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
std_string *hookFonts(std_string *str1, const char *str2);

#endif