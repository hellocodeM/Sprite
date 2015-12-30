#include "string.h"

void memcpy(void *dst, void *src, uint32_t size) {
    uint8_t* p = static_cast<uint8_t*>(dst);
    uint8_t* q = static_cast<uint8_t*>(src);
    for (uint32_t i = 0; i < size; i++) p[i] = q[i];
}

void memset(void *dst, uint8_t val, uint32_t size) {
    uint8_t* p = static_cast<uint8_t*>(dst);
    for (uint32_t i = 0; i < size; i++) p[i] = val;
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
