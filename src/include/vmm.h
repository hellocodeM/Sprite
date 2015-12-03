#ifndef VMM_H
#define VMM_H

#include "types.h"
#include "idt.h"

// 内核偏移地址
#define PAGE_OFFSET 0xC0000000

#define PMA(x) ((uint32_t)x - PAGE_OFFSET)
#define VMA(x) ((uint32_t)x + PAGE_OFFSET)

#define PAGE_PRESENT 0b1  // 有效位
#define PAGE_WRITE 0b10   // 可写位
#define PAGE_USER 0b100   // 用户位

// 虚拟页大小
#define PAGE_SIZE 4096

// 页掩码, 用于4k对齐
#define PAGE_MASK 0xFFFFF000

// 页目录掩码
#define PGD_INDEX(x) (((x) >> 22) & 0x3FF)

// 页表掩码
#define PTE_INDEX(x) (((x) >> 12) & 0x3FF)

// 页偏移掩码
#define OFFSET_INDEX(x) ((x)&0xFFF)

// 页目录数据类型
typedef uint32_t pde_t;

// 页表数据类型
typedef uint32_t pte_t;

// 页目录大小
#define PGD_SIZE 1024

// 页表大小
#define PTE_SIZE 1024

// 页表数量
#define PTE_COUNT 128

struct page_table {
    pte_t pages[PTE_SIZE];
};

struct page_directory {
    pde_t page_tables[PGD_SIZE];
};

// 内核页目录区域
extern pde_t pgd_kern[PGD_SIZE];

// 更换当前页目录
void switch_pgd(uint32_t pd);

// 映射一个虚拟页
uint32_t map(uint32_t va, uint32_t flags);

// 取消映射
void unmap(uint32_t va);

// 查找虚拟地址的映射
// 如果va映射到物理地址，返回1，同时把物理地址写入pa
uint32_t get_mapping(uint32_t va);

// 页错误的处理
void page_fault(pt_regs *regs);

// 初始化虚拟内存
void init_vmm();

#endif
