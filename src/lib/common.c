#include "common.h"

void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %1, %0" :: "dN" (port), "a" (value)) ;
}

uint8_t inb(uint16_t port) {
    uint8_t res;
    asm volatile ("inb %1, %0" : "=a" (res) : "dN" (port));
    return res;
}

uint16_t inw(uint16_t port) {
    uint16_t res;
    asm volatile ("inw %1, %0" : "=a" (res) : "dN" (port));
    return res;
}

void insb(uint32_t port, void* addr, uint32_t n) {
    asm volatile (
            "cld;"
            "repne; insb;"
            : "=D"(addr), "=c"(n)
            : "d"(port), "0"(addr), "l"(n)
            : "memory", "cc");
}

void insl(uint32_t port, void* addr, uint32_t n) {
    asm volatile (
            "cld;"
            "repne; insl;"
            : "=D"(addr), "=c"(n)
            : "d"(port), "0"(addr), "l"(n)
            : "memory", "cc");
}

void outsl(uint32_t port, const void* addr, int n) {
    asm volatile (
            "cld;"
            "repne; outsl;"
            : "=S"(addr), "=c"(n)
            : "d"(port), "0"(addr), "l"(n)
            : "memory", "cc");
}

void enable_intr() {
    asm volatile ("sti;");
}

void disable_intr() {
    asm volatile ("cli;");
}

void cpu_hlt() {
    asm volatile ("hlt;");
}
