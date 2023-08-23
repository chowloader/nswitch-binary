#include "chowjs.h"

int JS_SetPropertyStr(JSContext *ctx, JSValueConst this_obj, const char *prop, JSValue val){
    return 0;
}

JSValue JS_NewObject(JSContext *ctx){
    return 0;
}

JSValue JS_NewCFunction2(JSContext *ctx, JSCFunction *func, const char *name, int length, JSCFunctionEnum cproto, int magic){
    return 0;
}

void *qjs_malloc(JSMallocState *state, size_t size){
    return NULL;
}

void qjs_free(JSMallocState *state, void *ptr){
    return;
}

void *qjs_realloc(JSMallocState *state, void *ptr, size_t size){
    return ptr;
}

const char *JS_ToCStringLen2(JSContext *ctx, size_t *plen, JSValueConst val1, BOOL cesu8){
    return (const char *)ctx;
}

JSValue JS_NewStringLen(JSContext *ctx, const char *buf, size_t buf_len){
    return 0;
}

void JS_FreeCString(JSContext *ctx, const char* str){
    return;
}

JSValue get_aot_object(JSContext *ctx, int count, int* list){
    return 0;
}

JSValue JS_GetGlobalObject(JSContext *ctx){
    return 0;
}

JSValue JS_GetPropertyStr(JSContext *ctx, JSValueConst this_obj, const char*prop){
    return 0;
}

JSValue JS_NewArray(JSContext *ctx){
    return 0;
}

JSValue JS_DefinePropertyValueUint32(JSContext *ctx, JSValueConst this_obj, uint32_t index, JSValue val, int flags){
    return 0;
}

JSValue js_array_join(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int toLocaleString){
    return 0;
}

JSValue JS_ExecutePendingJob(JSRuntime *rt, JSContext **pctx){
    return 0;
}

JSValue js_global_decodeURI(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int isComponent){
    return 0;
}

void init_aot(JSContext *ctx){
    return;
}

JSValue JS_Call(JSContext *ctx, JSValueConst func_obj, JSValueConst this_obj, int argc, JSValueConst *argv){
    return 0;
}

int JS_ToInt32(JSContext *ctx, int32_t *pres, JSValueConst val){
    return 0;
}

int JS_ToInt64(JSContext *ctx, int64_t *pres, JSValueConst val){
    return 0;
}

JSRuntime *JS_NewRuntime(){
    return NULL;
}

JSContext *JS_NewContext(JSRuntime *rt){
    return NULL;
}

void JS_SetCanBlock(JSRuntime *rt, BOOL can_block){
    return;
}

void JS_FreeContext(JSContext *ctx){
    return;
}

void JS_FreeRuntime(JSRuntime *rt){
    return;
}

JSValue JS_Eval(JSContext *ctx, const char *input, size_t input_len, const char *filename, int eval_flags){
    return 0;
}

void *JS_GetOpaque(JSValueConst obj, JSClassID class_id){
    return NULL;
}

void build_backtrace(JSContext *ctx, JSValueConst error_obj, const char *filename, int line_num, int backtrace_flags){
    return;
}

JSValue JS_Throw(JSContext *ctx, JSValue obj){
    return 0;
}