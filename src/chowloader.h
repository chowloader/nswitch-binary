#ifndef __CHOWLOADER_H
#define __CHOWLOADER_H

#include "lib/imports.h"
#include "utils.h"

#define CHOWLOADER_DEBUG

int initChowLoader();
int initChowLoaderObject(JSContext *ctx, JSValueConst this_obj, const char *prop, JSValue val);
JSValue debugger(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue cacheImage(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue executeJobs(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue nextTick(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue nextTickJob(JSContext *ctx, int argc, JSValueConst *argv);

#endif