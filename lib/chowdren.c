#include "chowdren.h"

void BaseFile_initialize(void *bf, const char* file, const char* flag){
  return;
}

size_t BaseFile_get_size(void *bf){
  return 0;
}

void BaseFile_read(void *bf, void *buf, size_t size){
  return;
}

void BaseFile_close(void *bf){
  return;
}

size_t strlen(const char *s){
  return 0;
}

void *operator_new(size_t size){
  return NULL;
}

void operator_delete(void*ptr){
  return;
}

int stbi_info_from_memory(void * buffer, int len, int *x, int *y, int *comp){
  return 0;
}

void ChowdrenCacheImage(const char* path, int width, int height){
  return;
}

char *strcpy(char *destination, const char *source){
  return NULL;
}

int strcmp(const char* s1, const char *s2){
  return 0;
}

void platform_begin_draw(){
  return;
}

void Render_set_view(int x, int y, int width, int height){
  return;
}

void Render_clear(uint32_t color){
  return;
}

void platform_swap_buffers(){
  return;
}

void ImageUtils_update(){
  return;
}

uint32_t FontUtils_parse_color(const char *str){
  return 0;
}

void Thread_start(ChowdrenThread* this, int (*func)(void *), void *arg, char const* name){
  return;
}

bool Thread_isNull(ChowdrenThread* this){
  return false;
}

void Thread_join(ChowdrenThread* this){
  return;
}

void Thread_detach(ChowdrenThread* this){
  return;
}

void Thread_delete(ChowdrenThread* this){
  return;
}

void platform_sleep(double time){
  return;
}

void *cmemcpy(void *dest, void *src, size_t n){
  return NULL;
}

ChowdrenCachedImage *get_cached_image(int imageid){
  return NULL;
}

std_string *std_string_append(std_string *str1, const char *str2){
  return NULL;
}

stb_vorbis *stb_vorbis_open_memory(void *data, int len, int *error, void *alloc_buffer){
  return NULL;
}

void ChowdrenPreloadAudio(const char *p1, const char *p2, size_t file_size, size_t samples, unsigned int sample_rate, int channels){
  return;
}

int ImageUtils_get_image(std_string *path, void *imageid, void *width, void *height){
  return 0;
}

int64_t ImageHashTable = 1;
int64_t AudioPreloadHashTable = 1;
int64_t AudioHashTable = 1;

void *SearchImageHashTable(void *hash_table, std_string *path){
  return NULL;
}

void *SearchAudioPreloadHashTable(void *hash_table, std_string *path){
  return NULL;
}

void *SearchAudioHashTable(void *hash_table, std_string *path){
  return NULL;
}
