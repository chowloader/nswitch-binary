#include "../lib/imports.h"
#include "thread.h"

// !!! VERY EXPERIMENTAL !!!

int launchFunction(void *_args){
    void* thread = nn_os_GetCurrentThread();
    nn_os_SetThreadCoreMask(thread, -1, 7);

    ChowloaderThreadArgs *args = (ChowloaderThreadArgs*)_args;
    JSValue func = JS_Eval(args->ctx, args->eval_text, args->eval_size, "<threaded>", JS_EVAL_TYPE_GLOBAL);
    JS_Call(args->ctx, func, args->this_val, args->argc, args->argv);

    operator_delete(_args);

    return 1;
}

JSValue launchThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
    if(argc < 2) return JS_FALSE;

    ChowdrenThread thread;
    thread.thread = NULL;
    thread.weak = NULL;

    ChowloaderThreadArgs* args = operator_new(sizeof(ChowloaderThreadArgs));
    args->ctx = ctx;
    args->eval_text = JS_ToCStringLen2(ctx, &args->eval_size, argv[0], 0);
    args->this_val = argv[1];
    args->argc = argc - 2;
    args->argv = argv + 2;
    Thread_start(&thread, launchFunction, args, "ChowloaderThread");
    if(!Thread_isNull(&thread)){
        Thread_detach(&thread);
        return JS_TRUE;
    }
    return JS_FALSE;
}