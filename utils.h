#ifndef __UTILS_H
#define __UTILS_H

#include "lib/imports.h"

#define MAX(a,b) a>b?a:b
#define FLOOR(a) (int)a
#define CEIL(a) FLOOR(a)+1

std_string *to_std_string(const char *str);
const char *to_char_string(std_string *str);
size_t std_string_len(std_string *str);

const char* convertPath(JSContext *ctx, const char* path);
const char* convertPathJS(JSContext *ctx, JSValue str);
JSValue getChowloaderObject(JSContext *ctx);
JSValue getChowloaderNative(JSContext *ctx);
void emitChowloaderEventValue(JSContext *ctx, const char* event, JSValue val);
void emitChowloaderEvent(JSContext *ctx, const char* event);
void *readFile(JSContext *ctx, const char *cpath, size_t *size);

uint16_t swap16(uint16_t num);
uint32_t swap32(uint32_t num);
uint64_t swap64(uint64_t num);


#endif