#ifndef __CHOWLOADER_THREAD_H
#define __CHOWLOADER_THREAD_H

typedef struct ChowloaderThreadArgs {
    JSContext *ctx;
    const char* eval_text;
    size_t eval_size;
    JSValueConst this_val;
    int argc;
    JSValueConst *argv;
} ChowloaderThreadArgs;

int launchFunction(void *_args);
JSValue launchThread(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

#endif