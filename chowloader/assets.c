#include "assets.h"

JSValue createAssetsObject(JSContext *ctx){
  JSValue assets = JS_NewObject(ctx);

  JSValue _loadImage = JS_NewCFunction2(ctx, &loadImage, "loadImage", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, assets, "loadImage", _loadImage);

  JSValue _isImageLoaded = JS_NewCFunction2(ctx, &isImageLoaded, "isImageLoaded", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, assets, "isImageLoaded", _isImageLoaded);

  JSValue _isImagePreloaded = JS_NewCFunction2(ctx, &isImagePreloaded, "isImagePreloaded", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, assets, "isImagePreloaded", _isImagePreloaded);

  JSValue _loadAudio = JS_NewCFunction2(ctx, &loadAudio, "loadAudio", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, assets, "loadAudio", _loadAudio);

  JSValue _isAudioLoaded = JS_NewCFunction2(ctx, &isAudioLoaded, "isAudioLoaded", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, assets, "isAudioLoaded", _isAudioLoaded);

  JSValue _isAudioPreloaded = JS_NewCFunction2(ctx, &isAudioPreloaded, "isAudioPreloaded", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, assets, "isAudioPreloaded", _isAudioPreloaded);

  return assets;
}

JSValue loadImage(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  const char *path = convertPathJS(ctx, argv[0]);
  if(!path) return JS_FALSE;
  size_t size;
  void *image_buf = readFile(ctx, path, &size);
  int width;
  int height;
  stbi_info_from_memory(image_buf, size, &width, &height, NULL);
  qjs_free(&ctx->rt->malloc_state, image_buf);
  ChowdrenCacheImage(path, width, height);
  return JS_TRUE;
}

JSValue isImageLoaded(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  const char *path = convertPathJS(ctx, argv[0]);
  if(!path) return JS_FALSE;
  std_string *s = to_std_string(path);
  ChowdrenPreloadedImage *image = SearchImageHashTable(&ImageHashTable, s);
  operator_delete((void *)s->lstr.str);
  operator_delete((void *)s);
  if(!image) return JS_FALSE;
  if(IsImageLoading(image->imageId)) return JS_FALSE;
  return JS_TRUE;
}

JSValue isImagePreloaded(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  const char *path = convertPathJS(ctx, argv[0]);
  if(!path) return JS_FALSE;
  std_string *s = to_std_string(path);
  ChowdrenPreloadedImage *image = SearchImageHashTable(&ImageHashTable, s);
  operator_delete((void *)s->lstr.str);
  operator_delete((void *)s);
  if(!image) return JS_FALSE;
  return JS_TRUE;
}

JSValue loadAudio(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  const char *path = convertPathJS(ctx, argv[0]);
  if(!path) return JS_FALSE;
  size_t size;
  void *audio_buf = readFile(ctx, path, &size);
  int error;
  stb_vorbis *vorbis = stb_vorbis_open_memory(audio_buf, size, &error, NULL);
  if(!vorbis) return JS_FALSE;
  char *end_audio_buf = (char *)audio_buf + size - 6;
  while(end_audio_buf[0] != 'O' || end_audio_buf[1] != 'g' || end_audio_buf[2] != 'g' || end_audio_buf[3] != 'S' || end_audio_buf[4] || (end_audio_buf[5] & 0x4) == 0){
    if((uintptr_t)(--end_audio_buf) < (uintptr_t)audio_buf)
      return JS_FALSE;
  }
  uint32_t granulepos = *(uint32_t*)(end_audio_buf + 6);
  qjs_free(&ctx->rt->malloc_state, audio_buf);
  ChowdrenPreloadAudio(path+2, path, size, granulepos * vorbis->channels, vorbis->sample_rate, vorbis->channels);
  return JS_TRUE;
}

JSValue isAudioLoaded(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  const char *path = convertPathJS(ctx, argv[0]);
  if(!path) return JS_FALSE;
  std_string *s = to_std_string(path);
  void *audio = SearchAudioHashTable(&AudioHashTable, s);
  operator_delete((void *)s->lstr.str);
  operator_delete((void *)s);
  if(!audio) return JS_FALSE;
  return JS_TRUE;
}

JSValue isAudioPreloaded(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv){
  const char *path = convertPathJS(ctx, argv[0]);
  if(!path) return JS_FALSE;
  std_string *s = to_std_string(path);
  void *audio = SearchAudioPreloadHashTable(&AudioPreloadHashTable, s);
  operator_delete((void *)s->lstr.str);
  operator_delete((void *)s);
  if(!audio) return JS_FALSE;
  return JS_TRUE;
}

std_string *hookFonts(std_string *path, const char *ext){
  const char *str = to_char_string(path);
  if(str[8] == '.' && str[9] == '/'){
    str += 8;
  }
  size_t len = strlen(str);
  std_string *s;
  if(str[len - 4] == '.' && str[len - 3] == 't' && str[len - 2] == 't' && str[len - 1] == 'f'){
    s = to_std_string(str);
  } else {
    char *tmp = operator_new(len + 5);
    strcpy(tmp, str);
    strcpy(tmp + len, ext);
    s = to_std_string(tmp);
    operator_delete(tmp);
  }
  return s;
}

void hookBorder(std_string *path, void *imageid, void *width, void *height){
  char *str = (char *)to_char_string(path);
  if(str[13] == '.' && str[14] == '/'){
    str += 13;
  }
  size_t len = strlen(str);
  if(str[len - 8] == '.' && str[len - 7] == 'p' && str[len - 6] == 'n' && str[len - 5] == 'g'){
    len -= 4;
    str[len] = '\0';
  }
  std_string *s = to_std_string((const char *)str);
  ImageUtils_get_image(s, imageid, width, height);
  operator_delete((void *)s->lstr.str);
  operator_delete((void *)s);
}
