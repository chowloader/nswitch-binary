#ifndef __CHOWDREN_H
#define __CHOWDREN_H

#include <stdint.h>
#include <stdlib.h>
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
  uint8_t unk[32];
  int width;
  int height;
} ChowdrenCachedImage;

typedef struct ChowdrenPreloadedImage {
  uint8_t unk[40];
  uint32_t imageId;
} ChowdrenPreloadedImage;

typedef struct stb_vorbis {
  unsigned int sample_rate;
  int channels;
} stb_vorbis;

void BaseFile_initialize(void *bf, const char* file, const char* flag);
size_t BaseFile_get_size(void *bf);
void BaseFile_read(void *bf, void *buf, size_t size);
void BaseFile_close(void *bf);
size_t strlen(const char *s);
void *operator_new(size_t size);
void operator_delete(void*ptr);
int stbi_info_from_memory(void * buffer, int len, int *x, int *y, int *comp);
void ChowdrenCacheImage(const char* path, int width, int height);
char *strcpy(char *destination, const char *source);
int strcmp(const char* s1, const char *s2);
void platform_begin_draw();
void Render_set_view(int x, int y, int width, int height);
void Render_clear(uint32_t color);
void platform_swap_buffers();
void ImageUtils_update();
uint32_t FontUtils_parse_color(const char *str);
void Thread_start(ChowdrenThread* this, int (*func)(void *), void *arg, char const* name);
bool Thread_isNull(ChowdrenThread* this);
void Thread_join(ChowdrenThread* this);
void Thread_detach(ChowdrenThread* this);
void Thread_delete(ChowdrenThread* this);
void platform_sleep(double time);
void *cmemcpy(void *dest, void *src, size_t n);
ChowdrenCachedImage *get_cached_image(int imageid);
std_string *std_string_append(std_string *str1, const char *str2);
stb_vorbis *stb_vorbis_open_memory(void *data, int len, int *error, void *alloc_buffer);
void ChowdrenPreloadAudio(const char *p1, const char *p2, size_t file_size, size_t samples, unsigned int sample_rate, int channels);
int ImageUtils_get_image(std_string *path, void *imageid, void *width, void *height);
extern int64_t ImageHashTable;
extern int64_t AudioPreloadHashTable;
extern int64_t AudioHashTable;
ChowdrenPreloadedImage *SearchImageHashTable(void *hash_table, std_string *path);
void *SearchAudioPreloadHashTable(void *hash_table, std_string *path);
void *SearchAudioHashTable(void *hash_table, std_string *path);
int IsImageLoading(uint32_t id);

#endif