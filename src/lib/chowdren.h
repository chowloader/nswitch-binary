#ifndef __CHOWDREN_H
#define __CHOWDREN_H

#include <stdint.h>
#include <stdbool.h>

typedef struct basic_string_long {
  size_t length;
  size_t capacity;
  const char* str;
} basic_string_long;

typedef struct basic_string_short {
  unsigned char size;
  char str[0];
} basic_string_short;

typedef struct std_string {
  union {
    char first;
    basic_string_long lstr;
    basic_string_short sstr;
  };
} std_string;

typedef struct ChowdrenThread {
  void *thread;
  void *weak;
  bool unk;
} ChowdrenThread;

typedef struct ChowdrenCanvas {
  uint64_t unk;
  int width;
  int height;
} ChowdrenCanvas;

typedef struct ChowdrenCachedImage {
  uint32_t texture;
  uint8_t unk1[20];
  const char *filename;
  int width;
  int height;
  uint8_t unk2[16];
  void *pixels;
} ChowdrenCachedImage;

typedef struct ChowdrenPreloadedImage {
  uint8_t unk[40];
  uint32_t imageId;
} ChowdrenPreloadedImage;

typedef struct stb_vorbis {
  unsigned int sample_rate;
  int channels;
} stb_vorbis;

extern void BaseFile_initialize(void *bf, const char* file, const char* flag);
extern size_t BaseFile_get_size(void *bf);
extern void BaseFile_read(void *bf, void *buf, size_t size);
extern void BaseFile_close(void *bf);
extern size_t strlen(const char *s);
extern void *operator_new(size_t size);
extern void operator_delete(void*ptr);
extern int stbi_info_from_memory(void * buffer, int len, int *x, int *y, int *comp);
extern void ChowdrenCacheImage(const char* path, int width, int height);
extern char *strcpy(char *destination, const char *source);
extern int strcmp(const char* s1, const char *s2);
extern void platform_begin_draw();
extern void Render_set_view(int x, int y, int width, int height);
extern void Render_clear(uint32_t color);
extern void platform_swap_buffers();
extern void ImageUtils_update();
extern uint32_t FontUtils_parse_color(const char *str);
extern void Thread_start(ChowdrenThread* this, int (*func)(void *), void *arg, char const* name);
extern bool Thread_isNull(ChowdrenThread* this);
extern void Thread_join(ChowdrenThread* this);
extern void Thread_detach(ChowdrenThread* this);
extern void Thread_delete(ChowdrenThread* this);
extern void platform_sleep(double time);
extern void *cmemcpy(void *dest, void *src, size_t n);
extern ChowdrenCachedImage *get_cached_image(int imageid);
extern std_string *std_string_append(std_string *str1, const char *str2);
extern stb_vorbis *stb_vorbis_open_memory(void *data, int len, int *error, void *alloc_buffer);
extern void ChowdrenPreloadAudio(const char *p1, const char *p2, size_t file_size, size_t samples, unsigned int sample_rate, int channels);
extern int ImageUtils_get_image(std_string *path, void *imageid, void *width, void *height);
extern volatile int64_t ImageHashTable;
extern volatile int64_t AudioPreloadHashTable;
extern volatile int64_t AudioHashTable;
extern ChowdrenPreloadedImage *SearchImageHashTable(void *hash_table, std_string *path);
extern void *SearchAudioPreloadHashTable(void *hash_table, std_string *path);
extern void *SearchAudioHashTable(void *hash_table, std_string *path);
extern int IsImageLoading(uint32_t id);
extern uint8_t *stbi_zlib_decode_malloc(void *buffer, int len, int *outlen);

#endif