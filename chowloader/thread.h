#ifndef __CHOWLOADER_THREAD_H
#define __CHOWLOADER_THREAD_H

#include "../lib/imports.h"
#include "renderer.h"

typedef struct DummyOpaque {
  int64_t dummy;
  ChowdrenThread *thread;
  JSValue returnValue;
  int64_t isFinished;
} DummyOpaque;

typedef struct ChowloaderThreadArgs {
  JSContext *ctx;
  JSValueConst func;
  JSValueConst this_val;
  int argc;
  JSValueConst *argv;
  DummyOpaque* opaque;
} ChowloaderThreadArgs;

JSValue createThreadObject(JSContext *ctx);

extern JSClassID ThreadClassID;

void threadFinalizer(JSRuntime* rt, JSValue val);
JSValue launchThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue sleepThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue isFinishedThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue getReturnValueThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
JSValue stopThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
int launchFunction(void *_args);

#endif