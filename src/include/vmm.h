#ifndef VMM_H
#define VMM_H

#define PAGE_OFFSET 0xC0000000

#define PAGE_PRESENT 0b1
#define PAGE_WRITE 0b10
#define PAGE_USER 0b100

#define PAGE_MASK 0xFFFFF000
#define PGD_INDEX(x) ((x) >> 22 & 0x3FF)
#define PTE_INDEX(x) ((x) >> 12 & 0x3FF)

typedef uint32_t pde_t;
typedef uint32_t pte_t;
#endif
