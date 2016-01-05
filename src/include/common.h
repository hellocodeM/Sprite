#ifndef COMMON_H
#define COMMON_H

#include "types.h"

/**
 * 写一个字节
 */
void outb(uint16_t port, uint8_t value);

/** 
 * 读一个字节
 */
uint8_t inb(uint16_t port);

/** 
 * 读一个字
 */
uint16_t inw(uint16_t port);

void insb(uint32_t port, void* addr, uint32_t n);
/*
 * 读取若干uint32
 */
void insl(uint32_t port, void* addr, uint32_t n);

/**
 * 向端口写入字符串
 */
void outsl(uint32_t port, const void* addr, int n);

/**
 * 开启中断
 */
void enable_intr();

/**
 * 关闭中断
 */
void disable_intr();

/**
 * cpu 空转
 */
void cpu_hlt();

template <class T>
constexpr const T& min(const T& lhs, const T& rhs) {
    return lhs < rhs ? lhs : rhs;
}

template <class T>
constexpr const T& max(const T& lhs, const T& rhs) {
    return lhs < rhs ? rhs : lhs;
}


template <class L, class R>
struct is_same {
    const static bool value = false;
};

template <class T>
struct is_same<T, T> {
    const static bool value = true;
};

template <bool V>
struct enable_if;

template <>
struct enable_if<true> {};

#endif
