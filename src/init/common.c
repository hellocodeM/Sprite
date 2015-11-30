#include "common.h"

inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %1, %0" :: "dN" (port), "a" (value)) ;
}

inline uint8_t inb(uint16_t port) {
    uint8_t res;
    asm volatile ("inb %1, %0" : "=a" (res) : "dN" (port));
    return res;
}

inline uint16_t inw(uint16_t port) {
    uint16_t res;
    asm volatile ("inw %1, %0" : "=a" (res) : "dN" (port));
    return res;
}
