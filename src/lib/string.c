#include "string.h"

void memcpy(uint8_t *dst, uint8_t *src, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) dst[i] = src[i];
}

void memset(uint8_t *dst, uint8_t val, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) dst[i] = val;
}

void bzero(void *dst, uint32_t size) { memset((uint8_t*)dst, 0, size); }

int strcmp(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) return *str1 - *str2;
        ++str1;
        ++str2;
    }
    return *str1 - *str2;
}

char *strcpy(char *dst, const char *src) {
    while (*src) {
        *dst++ = *src++;
    }
    return dst;
}

char *strcat(char *dst, const char *src) {
    while (*dst++)
        ;
    return strcpy(dst, src);
}

const char *strfind(const char *str, char ch) {
    while (*str && *str != ch)
        ++str;
    return str;
}

int strlen(const char *src) {
    int len = 0;
    while (*src++) ++len;
    return len;
}
