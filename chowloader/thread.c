#include "thread.h"

JSValue createThreadObject(JSContext *ctx){
  JSValue thread = JS_NewObject(ctx);

  JSValue _launch = JS_NewCFunction2(ctx, &launchThread, "launch", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, thread, "launch", _launch);

  JSValue _sleep = JS_NewCFunction2(ctx, &sleepThread, "sleep", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, thread, "sleep", _sleep);

  JSValue _isFinished = JS_NewCFunction2(ctx, &isFinishedThread, "isFinished", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, thread, "isFinished", _isFinished);

  JSValue _getReturnValue = JS_NewCFunction2(ctx, &getReturnValueThread, "getReturnValue", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, thread, "getReturnValue", _getReturnValue);

  JSValue _stop = JS_NewCFunction2(ctx, &stopThread, "stop", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, thread, "stop", _stop);

  return thread;
}

JSClassID ThreadClassID = 1;

void threadFinalizer(JSRuntime* rt, JSValue val){
  DummyOpaque *opaque = JS_GetOpaque(val, ThreadClassID);
  if(!opaque) return;
  operator_delete(opaque);
}

JSValue launchThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  if(argc < 2) return JS_UNDEFINED;

  ChowdrenThread *thread = operator_new(sizeof(ChowdrenThread));
  thread->thread = NULL;
  thread->weak = NULL;

  DummyOpaque *opaque = operator_new(sizeof(DummyOpaque));
  opaque->returnValue = JS_UNINITIALIZED;
  opaque->isFinished = 0;

  ChowloaderThreadArgs *args = qjs_malloc(&ctx->rt->malloc_state, sizeof(ChowloaderThreadArgs));
  args->ctx = ctx;
  args->func = argv[0];
  args->this_val = argv[1];
  args->argc = argc - 2;
  args->argv = argv + 2;
  args->opaque = opaque;
  Thread_start(thread, launchFunction, args, "ChowloaderThread");
  if(!Thread_isNull(thread)){
    Thread_detach(thread);
    JSValue js_thread = JS_NewObjectClass(ctx, ThreadClassID);
    opaque->thread = thread;
    JS_SetOpaque(js_thread, opaque);
    return js_thread;
  }
  operator_delete(thread);
  operator_delete(opaque);
  qjs_free(&ctx->rt->malloc_state, args);
  return JS_UNDEFINED;
}

JSValue sleepThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  double time;
  if(argc != 0 && !JS_ToFloat64(ctx, &time, argv[0])){
    platform_sleep(time);
  }
  return JS_UNDEFINED;
}

JSValue isFinishedThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  if(argc == 0) return JS_FALSE;
  DummyOpaque *opaque = JS_GetOpaque(argv[0], ThreadClassID);
  if(!opaque) return JS_FALSE;
  return JS_MKVAL(JS_TAG_BOOL, opaque->isFinished);
}

JSValue getReturnValueThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  if(argc == 0) return JS_UNDEFINED;
  DummyOpaque *opaque = JS_GetOpaque(argv[0], ThreadClassID);
  if(opaque && opaque->isFinished) return opaque->returnValue;
  return JS_UNDEFINED;
}

JSValue stopThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  if(argc == 0) return JS_UNDEFINED;
  DummyOpaque *opaque = JS_GetOpaque(argv[0], ThreadClassID);
  if(!opaque) return JS_UNDEFINED;
  ChowdrenThread *thread = opaque->thread;
  if(thread && !Thread_isNull(thread)){
    Thread_delete(thread);
  }
  JSValue returnValue = JS_UNDEFINED;
  if(opaque->returnValue != JS_UNINITIALIZED) returnValue = opaque->returnValue;
  operator_delete(opaque);
  JS_SetOpaque(argv[0], NULL);
  return returnValue;
}

int launchFunction(void *_args){
  void* thread = nn_os_GetCurrentThread();
  nn_os_SetThreadCoreMask(thread, -1, 7);

  ChowloaderThreadArgs *args = (ChowloaderThreadArgs*)_args;
  args->opaque->returnValue = JS_Call(args->ctx, args->func, args->this_val, args->argc, args->argv);
  args->opaque->isFinished = 1;

  qjs_free(&args->ctx->rt->malloc_state, _args);

  return 1;
}
