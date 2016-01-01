#ifndef STRING_H
#define STRING_H

#include "types.h"

void memcpy(void *dst, void *src, uint32_t size);

void memset(void *dst, uint8_t val, uint32_t size);

void bzero(void *dst, uint32_t size);

int strcmp(const char *str1, const char *str2);

char *strcpy(char *dst, const char *src);

char *strcat(char *dst, const char *src);

const char* strfind(const char *str, char ch);

int strlen(const char *src);

#endif
