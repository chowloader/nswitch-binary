#include "utils.h"

std_string *to_std_string(const char *str){
  size_t len = strlen(str);
  std_string *s;
  if(len > 127){ // long
    s = operator_new(24);
    s->lstr.length = (len + 16) & 0xFFFFFFFFFFFFFFF0;
    s->lstr.capacity = len;
    s->lstr.str = operator_new(s->lstr.length);
    strcpy((char *)s->lstr.str, str);
    s->first |= 0x1;
  } else { // short
    s = operator_new(len + 2);
    s->sstr.size = len * 2;
    strcpy((char *)&s->sstr.str, str);
  }
  return s;
}

size_t std_string_len(std_string *str){
  if((str->first & 0x1) == 0){ // short
    return str->sstr.size / 2;
  } else { // long
    return str->lstr.capacity;
  }
}

const char *to_char_string(std_string *str){
  if((str->first & 0x1) == 0){ // short
    return (const char *)&str->sstr.str;
  } else { // long
    return str->lstr.str;
  }
}

const char* convertPath(JSContext *ctx, const char* path){
  if(path[0] == '.' && path[1] == '/') return path;
  if(path[0] == '/') path = path + 1;
  char* new_path = (char *)qjs_malloc(&ctx->rt->malloc_state, strlen(path) + 3); // "./" + path + NULL
  strcpy(new_path, "./");
  strcpy(new_path + 2, path);
  return new_path;
}

const char* convertPathJS(JSContext *ctx, JSValue str){
  //JSValue decoded_str = js_global_decodeURI(ctx, JS_UNDEFINED, 1, &str, 1);
  const char* path = JS_ToCStringLen2(ctx, NULL, str, 0);
  if(!path) return NULL;
  const char *new_path = convertPath(ctx, path);
  JS_FreeCString(ctx, path);
  return new_path;
}

JSValue getChowloaderObject(JSContext *ctx){
  return JS_GetPropertyStr(ctx, JS_GetGlobalObject(ctx), "chowloader");
}

JSValue getChowloaderNative(JSContext *ctx){
  JSValue chowloader = getChowloaderObject(ctx);
  return JS_GetPropertyStr(ctx, chowloader, "natives");
}

void emitChowloaderEventValue(JSContext *ctx, const char* event, JSValue val){
  JSValue chowloader = getChowloaderObject(ctx);
  JSValue emit_func = JS_GetPropertyStr(ctx, chowloader, "emit");
  JSValue* args = qjs_malloc(&ctx->rt->malloc_state, 16);
  args[0] = JS_NewStringLen(ctx, event, strlen(event));
  args[1] = val;
  JS_Call(ctx, emit_func, chowloader, 2, args);
  qjs_free(&ctx->rt->malloc_state, args);
}

void emitChowloaderEvent(JSContext *ctx, const char* event){
  emitChowloaderEventValue(ctx, event, JS_UNDEFINED);
}

void *readFile(JSContext *ctx, const char *cpath, size_t *size){
  const char* path = convertPath(ctx, cpath);
  if(!path) return NULL;
  void* basefile = qjs_malloc(&ctx->rt->malloc_state, 48);
  BaseFile_initialize(basefile, path, "r");
  *size = BaseFile_get_size(basefile);
  void * buf = qjs_malloc(&ctx->rt->malloc_state, *size);
  BaseFile_read(basefile, buf, *size);
  BaseFile_close(basefile);
  qjs_free(&ctx->rt->malloc_state, basefile);
  return buf;
}