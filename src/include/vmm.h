#ifndef VMM_H
#define VMM_H

#include "types.h"
#include "idt.h"

// 内核偏移地址
#define PAGE_OFFSET 0xC0000000

// 有效位
#define PAGE_PRESENT 0x1

// 可写位
#define PAGE_WRITE 0x2

// 用户位
#define PAGE_USER 0x4

// 虚拟页大小
#define PAGE_SIZE 4096

// 页掩码, 用于4k对齐
#define PAGE_MASK 0xFFFFF000

// 页目录掩码
#define PGD_INDEX(x) (((x) >> 22) & 0x3FF)

// 页表掩码
#define PTE_INDEX(x) (((x) >> 12) && 0x3FF)

// 页偏移掩码
#define OFFSET_INDEX(x) ((x) & 0xFFF)

// 页目录数据类型
typedef uint32_t pgd_t;

// 页表数据类型
typedef uint32_t pte_t;

// ???
#define PGD_SIZE (PAGE_SIZE/sizeof(pgd_t))

// ???
#define PTE_SIZE (PAGE_SIZE/sizeof(pte_t))

// 页表数量
#define PTE_COUNT 128

// 内核页目录区域
extern pgd_t pgd_kern[PGD_SIZE];

// 更换当前页目录
void switch_pgd(uint32_t pd);

// 使用flags之处当前的页权限，把物理地址映射到虚拟地址
void map(pgd_t *pgd_now, uint32_t va, uint32_t pa, uint32_t flags);

// 取消映射
void unmap(pgd_t *pgd_now, uint32_t va);

// 查找虚拟地址的映射
// 如果va映射到物理地址，返回1，同时把物理地址写入pa
uint32_t get_mapping(pgd_t *pgd_now, uint32_t va, uint32_t *pa);

// 页错误的处理
void page_fault(pt_regs *regs);


// 初始化虚拟内存
void init_vmm();

#endif 
