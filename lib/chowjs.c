#include "chowjs.h"

JSContext* ChowJSContext = (void*)(1);
JSRuntime* ChowJSRuntime = (void*)(1);

int JS_SetPropertyStr(JSContext *ctx, JSValueConst this_obj, const char *prop, JSValue val){
  return 0;
}

JSValue JS_NewObject(JSContext *ctx){
  return 0;
}

JSValue JS_NewCFunction2(JSContext *ctx, JSCFunction *func, const char *name, int length, JSCFunctionEnum cproto, int magic){
  return 0;
}

void *native_qjs_malloc(JSMallocState *state, size_t size){
  return NULL;
}

void native_qjs_free(JSMallocState *state, void *ptr){
  return;
}

void *native_qjs_realloc(JSMallocState *state, void *ptr, size_t size){
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

JSClassID JS_NewClassID(JSClassID *pclass_id){
  return 0;
}

int JS_NewClass(JSRuntime *rt, JSClassID class_id, JSClassDef *class_def){
  return 0;
}

JSValue JS_NewObjectClass(JSContext *ctx, int class_id){
  return 0;
}

void JS_SetOpaque(JSValue obj, void *opaque){
  return;
}

int JS_ToFloat64(JSContext *ctx, double *pres, JSValueConst val){
  return 0;
}

int JS_EnqueueJob(JSContext *ctx, JSJobFunc *job_func, int argc, JSValueConst *argv){
  return 0;
}

int JS_IsFunction(JSContext *ctx, JSValueConst val){
  return 0;
}

// Semaphore Synchronization for Multithreading Allocation

int chowdren_main(){
  return 0;
}

atomic_int_fast64_t semaphore = 1;

void semaphore_wait(){
  while(!semaphore){};
  semaphore--;
}

void semaphore_signal(){
  semaphore++;
}

void *qjs_malloc(JSMallocState *state, size_t size){
  semaphore_wait();

  void *ptr = native_qjs_malloc(state, size);

  semaphore_signal();

  return ptr;
}

void qjs_free(JSMallocState *state, void *ptr){
  semaphore_wait();

  native_qjs_free(state, ptr);
  
  semaphore_signal();
}

void *qjs_realloc(JSMallocState *state, void *ptr, size_t size){
  semaphore_wait();

  void *nptr = native_qjs_realloc(state, ptr, size);

  semaphore_signal();

  return nptr;
}
