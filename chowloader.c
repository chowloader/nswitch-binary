#include "lib/imports.h"
#include "utils.h"

#include "chowloader/assets.h"
#include "chowloader/aot.h"
#include "chowloader/renderer.h"

#include "chowloader.h"

int initChowLoaderObject(JSContext *ctx, JSValueConst this_obj, const char *prop, JSValue val){
    JS_SetPropertyStr(ctx, this_obj, prop, val);

    JSValue chowloader = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, this_obj, "chowloader", chowloader);

    JSValue _executeJobs = JS_NewCFunction2(ctx, &executeJobs, "executeJobs", 1, JS_CFUNC_generic, 0);
    JS_SetPropertyStr(ctx, chowloader, "executeJobs", _executeJobs);

    JS_SetPropertyStr(ctx, chowloader, "assets", createAssetsObject(ctx));

    JS_SetPropertyStr(ctx, chowloader, "renderer", createRendererObject(ctx));

    JS_SetPropertyStr(ctx, chowloader, "aot", createAOTObject(ctx));

#ifdef __CHOWLOADER_THREAD_H
    JSValue _launchThread = JS_NewCFunction2(ctx, &launchThread, "launchThread", 1, JS_CFUNC_generic, 0);
    JS_SetPropertyStr(ctx, chowloader, "launchThread", _launchThread);
#endif

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
