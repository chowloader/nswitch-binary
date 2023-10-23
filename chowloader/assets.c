#include "assets.h"

JSValue createAssetsObject(JSContext *ctx){
  JSValue renderer = JS_NewObject(ctx);

  JSValue _loadImage = JS_NewCFunction2(ctx, &loadImage, "loadImage", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, renderer, "loadImage", _loadImage);

  JSValue _loadAudio = JS_NewCFunction2(ctx, &loadAudio, "loadAudio", 1, JS_CFUNC_generic, 0);
  JS_SetPropertyStr(ctx, renderer, "loadAudio", _loadAudio);

  return renderer;
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
