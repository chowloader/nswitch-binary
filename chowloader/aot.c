#include "../lib/imports.h"
#include "../utils.h"
#include "aot.h"

JSValue getAOTObject(JSContext *ctx, const char* type, int count, void *va_list){
    JSValue array = JS_NewArray(ctx);

    int offset = ((int*)va_list)[6];

    const char **list = va_list + offset;
    const char **overflow_list = ((const char ***)va_list)[0];

    int c = -offset / 8;

    int first_count = count;
    if(first_count > c) first_count = c;

    for(uint32_t i = 0; i < first_count; ++i){
        const char* str = list[i];
        JS_DefinePropertyValueUint32(ctx, array, i, JS_NewStringLen(ctx, str, strlen(str)), 7);
    }

    if(count > 5){
        for(uint32_t i = 0; i < count - c; ++i){
            const char* str = overflow_list[i];
            JS_DefinePropertyValueUint32(ctx, array, i+c, JS_NewStringLen(ctx, str, strlen(str)), 7);
        }
    }

    JSValue aot_object = get_aot_object(ctx, count, va_list);

    JSValue object = JS_NewObject(ctx);
    
    JS_SetPropertyStr(ctx, object, "type", JS_NewStringLen(ctx, type, strlen(type)));
    JS_SetPropertyStr(ctx, object, "object", aot_object);

    JSValue joinstr = JS_NewStringLen(ctx, ".", 1);
    JSValue path = js_array_join(ctx, array, 1, &joinstr, 0);

    JS_SetPropertyStr(ctx, object, "path", path);

    emitChowloaderEventValue(ctx, "aot_object", object);

    return aot_object;
}

JSValue hookJSAOT(JSContext *ctx, int count, void *va_list){
    return getAOTObject(ctx, "jsaot", count, va_list);
}

JSValue hookJSVAL(JSContext *ctx, int count, void *va_list){
    return getAOTObject(ctx, "jsval", count, va_list);
}

JSValue hookJSVARREF(JSContext *ctx, int count, void *va_list){
    return getAOTObject(ctx, "jsval", count, va_list);
}

void initAOT(JSContext *ctx){
    emitChowloaderEvent(ctx, "omori_loaded");
    init_aot(ctx);
    emitChowloaderEvent(ctx, "aot_loaded");
}

// AOT Patching

JSValue createAOTObject(JSContext *ctx){
    JSValue aot = JS_NewObject(ctx);

    JSValue _findJSVALNative = JS_NewCFunction2(ctx, &findJSVALNative, "find", 1, JS_CFUNC_generic, 0);
    JS_SetPropertyStr(ctx, aot, "find", _findJSVALNative);

    JSValue _patchJSVALNative = JS_NewCFunction2(ctx, &patchJSVALNative, "patch", 1, JS_CFUNC_generic, 0);
    JS_SetPropertyStr(ctx, aot, "patch", _patchJSVALNative);

    return aot;
}

JSValue JSVALOffset = 0;

JSValue findJSVALNative(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
    JSValue* offset = &JSVALOffset;

    for(size_t i = 0; i < JS_VALUE_GET_UINT(argv[0]); i++){
        if(offset[i] == argv[1]){
            return JS_MKVAL(JS_TAG_INT, (uintptr_t)(offset + i));
        }
    }

    return JS_MKVAL(JS_TAG_INT, 0xFFFFFFFF);
}

JSValue patchJSVALNative(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
    JSValue* val = (JSValue*)JS_VALUE_GET_UINT(argv[0]);
    val[0] = argv[1];
    return JS_TRUE;
}