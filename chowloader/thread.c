#include "thread.h"

JSValue createThreadObject(JSContext *ctx){
  JSValue thread = JS_NewObject(ctx);

  JSValue _launch = JS_NewCFunction2(ctx, &launchThread, "launch", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, thread, "launch", _launch);

  JSValue _stop = JS_NewCFunction2(ctx, &stopThread, "stop", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, thread, "stop", _launch);

  return thread;
}

JSValue launchThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  if(argc < 2) return JS_UNDEFINED;

  ChowdrenThread *thread = operator_new(sizeof(ChowdrenThread));
  thread->thread = NULL;
  thread->weak = NULL;

  ChowloaderThreadArgs *args = qjs_malloc(&ctx->rt->malloc_state, sizeof(ChowloaderThreadArgs));
  args->ctx = ctx;
  args->func = argv[0];
  args->this_val = argv[1];
  args->argc = argc - 2;
  args->argv = argv + 2;
  Thread_start(thread, launchFunction, args, "ChowloaderThread");
  if(!Thread_isNull(thread)){
    Thread_detach(thread);
    JSValue js_thread = JS_NewObjectClass(ctx, CanvasClassID);
    DummyOpaque *opaque = operator_new(sizeof(DummyOpaque));
    opaque->thread = thread;
    JS_SetOpaque(js_thread, opaque);
    return js_thread;
  }
  return JS_UNDEFINED;
}

JSValue stopThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  if(argc == 0) return JS_UNDEFINED;
  DummyOpaque *opaque = JS_GetOpaque(argv[0], CanvasClassID);
  ChowdrenThread *thread = opaque->thread;
  if(thread && !Thread_isNull(thread)){
    Thread_delete(thread);
  }
  operator_delete(opaque);
  JS_SetOpaque(argv[0], NULL);
  return JS_UNDEFINED;
}

int launchFunction(void *_args){
  void* thread = nn_os_GetCurrentThread();
  nn_os_SetThreadCoreMask(thread, -1, 7);

  ChowloaderThreadArgs *args = (ChowloaderThreadArgs*)_args;
  JS_Call(args->ctx, args->func, args->this_val, args->argc, args->argv);

  qjs_free(&args->ctx->rt->malloc_state, _args);

  return 1;
}
