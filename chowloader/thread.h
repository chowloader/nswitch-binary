#ifndef __CHOWLOADER_THREAD_H
#define __CHOWLOADER_THREAD_H

#include "../lib/imports.h"
#include "renderer.h"

typedef struct ChowloaderThreadArgs {
  JSContext *ctx;
  JSValueConst func;
  JSValueConst this_val;
  int argc;
  JSValueConst *argv;
} ChowloaderThreadArgs;

typedef struct DummyOpaque {
  int64_t placeholder1;
  int64_t placeholder2;
  int64_t placeholder3;
  ChowdrenThread *thread;
} DummyOpaque;

extern double ms_divide;

JSValue createThreadObject(JSContext *ctx);
JSValue launchThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue sleepThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue stopThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
int launchFunction(void *_args);

#endif