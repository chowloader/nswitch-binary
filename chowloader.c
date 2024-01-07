#include "chowloader.h"

#include "chowloader/assets.h"
#include "chowloader/aot.h"
#include "chowloader/renderer.h"
#include "chowloader/thread.h"

int initChowLoader(){
  semaphore = 1;
  return chowdren_main();
}

int initChowLoaderObject(JSContext *ctx, JSValueConst this_obj, const char *prop, JSValue val){
  JS_NewClassID(&ThreadClassID);
  JSClassDef *thread_def = operator_new(sizeof(thread_def));
  thread_def->class_name = "ThreadHandle";
  thread_def->finalizer = threadFinalizer;
  thread_def->gc_mark = NULL;
  thread_def->call = NULL;
  thread_def->exotic = NULL;
  JS_NewClass(ctx->rt, ThreadClassID, thread_def);

  JS_SetPropertyStr(ctx, this_obj, prop, val);

  JSValue chowloader = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, this_obj, "chowloader", chowloader);

  JSValue _executeJobs = JS_NewCFunction2(ctx, &executeJobs, "executeJobs", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, chowloader, "executeJobs", _executeJobs);

  JS_SetPropertyStr(ctx, chowloader, "assets", createAssetsObject(ctx));

  JS_SetPropertyStr(ctx, chowloader, "renderer", createRendererObject(ctx));

  JS_SetPropertyStr(ctx, chowloader, "aot", createAOTObject(ctx));

  JS_SetPropertyStr(ctx, chowloader, "thread", createThreadObject(ctx));

#ifdef CHOWLOADER_DEBUG
  JSValue _debugger = JS_NewCFunction2(ctx, &debugger, "debugger", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, chowloader, "debugger", _debugger);
#endif

  return 1;
}

#ifdef CHOWLOADER_DEBUG
JSValue debugger(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  __builtin_debugtrap();
  return JS_UNDEFINED;
}
#endif

JSValue executeJobs(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  JS_ExecutePendingJob(ctx->rt, &ctx);
  return JS_UNDEFINED;
}

int main(){
  return 0;
}
