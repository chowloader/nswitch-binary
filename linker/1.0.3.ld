MEMORY
{
  exec (rx) : ORIGIN = 0xBAB3FE8, LENGTH = 78296
  data (rw) : ORIGIN = 0xC13AA00, LENGTH = 1M
}

SECTIONS
{
  .text : ALIGN(4)
  {
    *(.text*)
    *(.rodata*)
  } > exec

  .data : ALIGN(1)
  {
    *(.data*)
    *(.bss*)
  } > data

  /* We don't need an entry function, this remove the linker warning */
  _start = 0;

  /* OMORI Functions */
  JS_SetPropertyStr = 0xBADF670;
  JS_NewObject = 0xBAD6000;
  JS_NewCFunction2 = 0xBAD6110;
  native_qjs_malloc = 0xBBA9330;
  native_qjs_free = 0xBBA94D0;
  native_qjs_realloc = 0xBBA9550;
  operator_new = 0xBCDD470;
  JS_ToCStringLen2 = 0xBAD51E0;
  JS_NewStringLen = 0xBAD3250;
  strlen = 0xBCDD060;
  JS_FreeCString = 0xBAD5570;
  BaseFile_initialize = 0xBC5C9AC;
  BaseFile_get_size = 0xBC5CCB4;
  BaseFile_read = 0xBC5D040;
  BaseFile_close = 0xBC5CC4C;
  stbi_info_from_memory = 0x808C1F0;
  ChowdrenCacheImage = 0x809A660;
  operator_delete = 0xBCDCFF0;
  get_aot_object = 0xBB9D940;
  JS_GetGlobalObject = 0xBAD7D10;
  JS_GetPropertyStr = 0xBADB890;
  JS_NewArray = 0xBAD5FE0;
  JS_DefinePropertyValueUint32 = 0xBADFD90;
  js_array_join = 0xBB8A860;
  JS_ExecutePendingJob = 0xBACEAB0;
  js_global_decodeURI = 0xBB8E250;
  strcpy = 0xBCDD770;
  strcmp = 0xBCDD2E0;
  init_aot = 0xBB07720;
  JS_Call = 0xBAF3C30;
  platform_begin_draw = 0xBC5A6F0;
  Render_set_view = 0xBC5E770;
  Render_clear = 0xBC5FC70;
  ImageUtils_update = 0x809A9B0;
  platform_swap_buffers = 0xBC5A930;
  FontUtils_parse_color = 0x80B2030;
  JS_ToInt32 = 0xBAE25B0;
  JS_ToInt64 = 0xBAE22E0;
  Thread_start = 0x8005254;
  Thread_isNull = 0xBCDD0B0;
  Thread_join = 0xBCDD120;
  Thread_detach = 0xBCDD440;
  Thread_delete = 0xBCDD0D0;
  platform_sleep = 0xBC59A20;
  nn_os_GetCurrentThread = 0xBCDD170;
  nn_os_SetThreadCoreMask = 0xBCDD180;
  JS_NewRuntime = 0xBACE940;
  JS_NewContext = 0xBACFD70;
  JS_SetCanBlock = 0xBACE980;
  JS_FreeContext = 0xBAD2980;
  JS_FreeRuntime = 0xBACE690;
  cmemcpy = 0xBCDD070;
  JS_Eval = 0xBAF6DD0;
  JS_GetOpaque = 0xBAE1580;
  get_cached_image = 0x80AF860;
  JS_Throw = 0xBAD7D40;
  build_backtrace = 0xBAF4D00;
  std_string_append = 0xBCDD500;
  stb_vorbis_open_memory = 0x80BF850;
  ChowdrenPreloadAudio = 0x80D0300;
  chowdren_main = 0x8089BB0;
  JS_ToFloat64 = 0xBAE1B70;
  ImageUtils_get_image = 0x808C820;
  JS_NewObjectClass = 0xBAD5E50;
  JS_NewClassID = 0xBAD4730;
  JS_NewClass = 0xBAD4790;
  JS_SetOpaque = 0xBAD66B0;
  JS_EnqueueJob = 0xBACE9A0;
  JS_IsFunction = 0xBAD8CB0;
  SearchImageHashTable = 0x80AFA08;
  SearchAudioPreloadHashTable = 0x80D6188;
  SearchAudioHashTable = 0x80D8BC0;
  IsImageLoading = 0x809A980;
  stbi_zlib_decode_malloc = 0x808BF70;

  /* OMORI Data */
  isDrawing = 0xBF27355;
  Render_offset = 0x10542300;
  Render_offsetf = 0x10542348;
  ChowJSRuntime = 0xC136AC0;
  ChowJSContext = 0xC136AB8;
  CanvasClassID = 0xC136A8C;
  JSVALOffset = 0xC131E10;
  ImageHashTable = 0xC0ADF48;
  AudioHashTable = 0xC0C9138;
  AudioPreloadHashTable = 0xC0C90E8;
}