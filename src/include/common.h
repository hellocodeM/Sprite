#ifndef COMMON_H
#define COMMON_H

#include "types.h"

// 写一个字节
void outb(uint16_t port, uint8_t value);

// 读一个字节
uint8_t inb(uint16_t port);

// 读一个字
uint16_t inw(uint16_t port);

#endif
