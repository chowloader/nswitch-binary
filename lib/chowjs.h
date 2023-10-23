#ifndef __CHOWJS_H
#define __CHOWJS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>

#define JS_EVAL_TYPE_GLOBAL   (0 << 0)
#define JS_EVAL_TYPE_MODULE   (1 << 0)
#define JS_EVAL_TYPE_DIRECT   (2 << 0)
#define JS_EVAL_TYPE_INDIRECT (3 << 0)

#define JS_EVAL_FLAG_STRICT            (1 << 3)
#define JS_EVAL_FLAG_STRIP             (1 << 4)
#define JS_EVAL_FLAG_COMPILE_ONLY      (1 << 5)
#define JS_EVAL_FLAG_BACKTRACE_BARRIER (1 << 6)

typedef uint64_t JSValue;
typedef JSValue JSValueConst;
typedef uint64_t BOOL;
typedef uint64_t JS_BOOL;
typedef uint32_t JSAtom;
typedef uint32_t JSClassID;

struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

typedef struct JSMallocState {
    size_t malloc_count;
    size_t malloc_size;
    size_t malloc_limit;
    void *opaque;
} JSMallocState;

typedef struct JSMallocFunctions {
    void *(*js_malloc)(JSMallocState *s, size_t size);
    void (*js_free)(JSMallocState *s, void *ptr);
    void *(*js_realloc)(JSMallocState *s, void *ptr, size_t size);
    size_t (*js_malloc_usable_size)(const void *ptr);
} JSMallocFunctions;

typedef struct JSRuntime_Atoms {
    int atom_hash_size;
    int atom_count;
    int atom_size;
    int atom_count_resize;
    uint32_t *atom_hash;
    void **atom_array;
    int atom_free_index;
} JSRuntime_Atoms;

typedef struct JSRuntime {
    JSMallocFunctions mf;
    JSMallocState malloc_state;
    const char *rt_info;

    JSRuntime_Atoms atoms;
} JSRuntime;

typedef struct JSContext {
    char header[0x18];
    JSRuntime *rt;
    struct list_head link;

    uint16_t binary_object_count;
    int binary_object_size;

    void *array_shape;

    JSValue *class_proto;
    JSValue function_proto;
    JSValue function_ctor;
    JSValue array_ctor;
    JSValue regexp_ctor;
    JSValue promise_ctor;
    JSValue native_error_proto[8];
    JSValue iterator_proto;
    JSValue async_iterator_proto;
    JSValue array_proto_values;
    JSValue throw_type_error;
    JSValue eval_obj;

    JSValue global_obj;
    JSValue global_var_obj;
} JSContext;

typedef void JSClassFinalizer(JSRuntime *rt, JSValue val);

typedef struct JSClassDef {
  const char *class_name;
  JSClassFinalizer *finalizer;
  void *gc_mark;
  void *call;
  void *exotic;
} JSClassDef;

typedef JSValue JSCFunction(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

typedef enum JSCFunctionEnum {
    JS_CFUNC_generic,
    JS_CFUNC_generic_magic,
    JS_CFUNC_constructor,
    JS_CFUNC_constructor_magic,
    JS_CFUNC_constructor_or_func,
    JS_CFUNC_constructor_or_func_magic,
    JS_CFUNC_f_f,
    JS_CFUNC_f_f_f,
    JS_CFUNC_getter,
    JS_CFUNC_setter,
    JS_CFUNC_getter_magic,
    JS_CFUNC_setter_magic,
    JS_CFUNC_iterator_next
} JSCFunctionEnum;

enum {
    JS_TAG_UNINITIALIZED = 0,
    JS_TAG_INT = 1,
    JS_TAG_BOOL = 2,
    JS_TAG_NULL = 3,
    JS_TAG_UNDEFINED = 4,
    JS_TAG_CATCH_OFFSET = 5,
    JS_TAG_EXCEPTION = 6,
    JS_TAG_FLOAT64 = 7,

    JS_TAG_OBJECT = 8,
    JS_TAG_FUNCTION_BYTECODE = 9,
    JS_TAG_MODULE = 10,
    JS_TAG_STRING = 11,
    JS_TAG_SYMBOL = 12,
    JS_TAG_BIG_FLOAT = 13,
    JS_TAG_BIG_INT = 14,
    JS_TAG_BIG_DECIMAL = 15
};

#define JS_VALUE_GET_TAG(v) (uint8_t)((v) >> 48)
#define JS_VALUE_GET_INT(v) (int64_t)(v & 0xFFFFFFFFFFFFL)
#define JS_VALUE_GET_UINT(v) (uint64_t)(v & 0xFFFFFFFFFFFFL)
#define JS_VALUE_GET_BOOL(v) (int64_t)(v & 0xFFFFFFFFFFFFL)
#define JS_VALUE_GET_PTR(v) (void *)(v & 0xFFFFFFFFFFFFL)

#define JS_MKVAL(tag, val) (((uint64_t)(tag) << 48) | (uint64_t)(val))
#define JS_MKPTR(tag, ptr) (((uint64_t)(tag) << 48) | ((uintptr_t)(ptr) & 0xFFFFFFFFFFFFL))

#define JS_UNINITIALIZED JS_MKVAL(JS_TAG_UNINITIALIZED, 0)
#define JS_FALSE JS_MKVAL(JS_TAG_BOOL, 0)
#define JS_TRUE JS_MKVAL(JS_TAG_BOOL, 1)
#define JS_NULL JS_MKVAL(JS_TAG_NULL, 0)
#define JS_UNDEFINED JS_MKVAL(JS_TAG_UNDEFINED, 0)
#define JS_EXCEPTION JS_MKVAL(JS_TAG_EXCEPTION, 0)

extern JSContext* ChowJSContext;
extern JSRuntime* ChowJSRuntime;

int JS_SetPropertyStr(JSContext *ctx, JSValueConst this_obj, const char *prop, JSValue val);
JSValue JS_NewObject(JSContext *ctx);
JSValue JS_NewCFunction2(JSContext *ctx, JSCFunction *func, const char *name, int length, JSCFunctionEnum cproto, int magic);
void *native_qjs_malloc(JSMallocState *state, size_t size);
void native_qjs_free(JSMallocState *state, void *ptr);
void *native_qjs_realloc(JSMallocState *state, void *ptr, size_t size);
const char *JS_ToCStringLen2(JSContext *ctx, size_t *plen, JSValueConst val1, BOOL cesu8);
JSValue JS_NewStringLen(JSContext *ctx, const char *buf, size_t buf_len);
void JS_FreeCString(JSContext *ctx, const char* str);
JSValue get_aot_object(JSContext *ctx, int count, int* list);
JSValue JS_GetGlobalObject(JSContext *ctx);
JSValue JS_GetPropertyStr(JSContext *ctx, JSValueConst this_obj, const char*prop);
JSValue JS_NewArray(JSContext *ctx);
JSValue JS_DefinePropertyValueUint32(JSContext *ctx, JSValueConst this_obj, uint32_t index, JSValue val, int flags);
JSValue js_array_join(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int toLocaleString);
JSValue JS_ExecutePendingJob(JSRuntime *rt, JSContext **pctx);
JSValue js_global_decodeURI(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int isComponent);
void init_aot(JSContext *ctx);
JSValue JS_Call(JSContext *ctx, JSValueConst func_obj, JSValueConst this_obj, int argc, JSValueConst *argv);
int JS_ToInt32(JSContext *ctx, int32_t *pres, JSValueConst val);
int JS_ToInt64(JSContext *ctx, int64_t *pres, JSValueConst val);
JSRuntime *JS_NewRuntime();
JSContext *JS_NewContext(JSRuntime *rt);
void JS_SetCanBlock(JSRuntime *rt, BOOL can_block);
void JS_FreeContext(JSContext *ctx);
void JS_FreeRuntime(JSRuntime *rt);
JSValue JS_Eval(JSContext *ctx, const char *input, size_t input_len, const char *filename, int eval_flags);
void *JS_GetOpaque(JSValueConst obj, JSClassID class_id);
void build_backtrace(JSContext *ctx, JSValueConst error_obj, const char *filename, int line_num, int backtrace_flags);
JSValue JS_Throw(JSContext *ctx, JSValue obj);
JSClassID JS_NewClassID(JSClassID *pclass_id);
int JS_NewClass(JSRuntime *rt, JSClassID class_id, JSClassDef *class_def);
JSValue JS_NewObjectClass(JSContext *ctx, int class_id);
void JS_SetOpaque(JSValue obj, void *opaque);
int JS_ToFloat64(JSContext *ctx, double *pres, JSValueConst val);

// Semaphore Synchronization for Multithreading Allocation

int chowdren_main();

extern atomic_int_fast64_t semaphore;

int semaphore_init();
void semaphore_wait();
void semaphore_signal();

void *qjs_malloc(JSMallocState *state, size_t size);
void qjs_free(JSMallocState *state, void *ptr);
void *qjs_realloc(JSMallocState *state, void *ptr, size_t size);

#endif