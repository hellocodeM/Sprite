#ifndef PMM_H
#define PMM_H

#include "multiboot.h"

// 内核的起始和结束地址
extern uint8_t kern_start[];
extern uint8_t kern_end[];

// 线程栈的大小
#define STACK_SIZE 0x1000

// 支持的物理内存大小
#define PMM_MAX_SIZE 0x20000000

// 物理页大小
#define PMM_PAGE_SIZE 0x1000

// 支持的物理页数量
#define PAGE_MAX_SIZE (PMM_MAX_SIZE/PMM_PAGE_SIZE)

// 页表页码
#define PHY_PAGE_MASK 0xFFFFF000

// 分配的物理页的总数
extern uint32_t phy_page_count;

void init_pmm();

// 申请内存
uint32_t pmm_alloc_page();

uint32_t pmm_alloc_page(uint32_t pma);

// 释放内存
void pmm_free_page(uint32_t p);

// 显示内存分布
void show_mmap();

void show_kern_mmap();

#endif
