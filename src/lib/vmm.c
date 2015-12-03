#include "idt.h"
#include "string.h"
#include "debug.h"
#include "vmm.h"
#include "pmm.h"
#include "printk.h"

// 内核页目录区域
pde_t pgd_kern[PGD_SIZE] __attribute__((aligned(PAGE_SIZE)));

// 内核页表区域
static pte_t pte_kern[PTE_COUNT][PTE_SIZE] __attribute__((aligned(PAGE_SIZE)));

void init_vmm() {
    /*
    // 建立内核VMA的固定映射
    // 0xC0000000 这个地址在页目录的索引
    uint32_t kern_pte_first_idx = PGD_INDEX(PAGE_OFFSET);
    uint32_t i, j;
    for (i = kern_pte_first_idx, j = 0; i < PTE_COUNT + kern_pte_first_idx; i++, j++) {
        // 此处是内核虚拟地址，MMU 需要物理地址，所以减去偏移，下同
        pgd_kern[i] = ((uint32_t)PMA(pte_kern[j])) | PAGE_PRESENT | PAGE_WRITE;
    }

    uint32_t *pte = (uint32_t *)pte_kern;
    // 不映射第 0 页，便于跟踪 NULL 指针
    for (i = 1; i < PTE_COUNT * PTE_SIZE; i++) {
        pte[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
    }

    // 注册页错误中断的处理函数 ( 14 是页故障的中断号 )
    register_isr(14, &page_fault);

    // 切换页目录
    uint32_t pgd_kern_phy_addr = PMA(pgd_kern);
    switch_pgd(pgd_kern_phy_addr);
    */

    // 固定映射0xC0000000~0xC0400000的内核空间
    uint32_t pgd_index = PGD_INDEX(PAGE_OFFSET);
    uint32_t pgt_index = PTE_INDEX(PAGE_OFFSET);
    pgd_kern[pgd_index] = PMA(pte_kern[pgt_index]) | PAGE_PRESENT | PAGE_WRITE;
    
    extern pde_t *pte_low;
    for (int i = 0; i < 1024; i++) {
        pte_kern[pgt_index][i] = pte_low[i] | PAGE_PRESENT | PAGE_WRITE;
    }

    register_isr(14, &page_fault);
    uint32_t pgd = PMA(pgd_kern);
    switch_pgd(pgd);
}

void switch_pgd(uint32_t pd) { asm volatile("mov %0, %%cr3" : : "r"(pd)); }

static pte_t &find_pte(uint32_t vma) {
    pde_t *page_directory;
    asm volatile("mov %%cr3, %0;" : : "r"(page_directory));
    uint32_t pde_index = PGD_INDEX(vma);
    uint32_t pte_index = PTE_INDEX(vma);

    pde_t pde = page_directory[pde_index] & PAGE_MASK;
    if (!(pde & PAGE_PRESENT)) {
        assert(false, "page table is not present");
    }
    pte_t *page_table = reinterpret_cast<pte_t *>(pde);

    return page_table[pte_index];
}

inline static void flush_page_table_cache(uint32_t vma) {
    asm volatile("invlpg (%0)" : : "a"(vma));
}

uint32_t map(uint32_t vma, uint32_t flags) {
    /*
    uint32_t pgd;
    asm volatile ("mov %%cr3, %0;" : : "r"(pgd));
    pde_t *pgd_now = (pde_t*)pgd;
    uint32_t pgd_idx = PGD_INDEX(va);
    uint32_t pte_idx = PTE_INDEX(va);

    pte_t *pte = (pte_t *)(pgd_now[pgd_idx] & PAGE_MASK);
    if (!pte) {
        pte = (pte_t *)pmm_alloc_page();
        pgd_now[pgd_idx] = (uint32_t)pte | PAGE_PRESENT | PAGE_WRITE;

        // 转换到内核线性地址并清 0
        pte = (pte_t *)((uint32_t)pte + PAGE_OFFSET);
        bzero(pte, PAGE_SIZE);
    } else {
        // 转换到内核线性地址
        pte = (pte_t *)((uint32_t)pte + PAGE_OFFSET);
    }

    pte[pte_idx] = (pa & PAGE_MASK) | flags;

    // 通知 CPU 更新页表缓存
    asm volatile("invlpg (%0)" : : "a"(va));
    */

    pte_t &pte = find_pte(vma);
    assert(!(pte & PAGE_PRESENT), "page should not present");
    uint32_t pma = pmm_alloc_page();
    pte = (pma & PAGE_MASK) | flags;

    flush_page_table_cache(vma);
    return pma;
}

void unmap(uint32_t va) {
    uint32_t pgd;
    asm volatile("mov %%cr3, %0;" : : "r"(pgd));
    pde_t *pgd_now = (pde_t *)pgd;
    uint32_t pgd_idx = PGD_INDEX(va);
    uint32_t pte_idx = PTE_INDEX(va);

    pte_t *pte = (pte_t *)(pgd_now[pgd_idx] & PAGE_MASK);

    if (!pte) {
        return;
    }

    // 转换到内核线性地址
    pte = (pte_t *)((uint32_t)pte + PAGE_OFFSET);

    pte[pte_idx] = 0;

    // 通知 CPU 更新页表缓存
    asm volatile("invlpg (%0)" : : "a"(va));
}

uint32_t get_mapping(uint32_t va) {
    pde_t *pgd;
    asm volatile("mov %%cr3, %0;" : : "r"(pgd));
    uint32_t pgd_idx = PGD_INDEX(va);
    uint32_t pte_idx = PTE_INDEX(va);

    pde_t *pde = (pde_t *)(pgd[pgd_idx] & PAGE_MASK);
    if (!pde) {
        return 0;
    }
    pte_t *pte = (pte_t *)(pde[pte_idx] & PAGE_MASK);
    if (!pte) {
        return 0;
    }
    return (uint32_t)pte & PAGE_MASK;
}

void page_fault(pt_regs *regs) {
    static int fault_cnt = 0;
    if (++fault_cnt > 5) {
        assert(false, "too many page_fault");
    }
    uint32_t addr;
    asm volatile("mov %%cr2, %0" : "=r"(addr));

    printk("virtual faulting address 0x%x\n", addr);
    printk("Error code: %x\n", regs->err_code);

    if (!(regs->err_code & PAGE_PRESENT)) {
        printk("Because the page wasn't present.\n");
    }
    if (regs->err_code & PAGE_WRITE) {
        printk("Write error.\n");
    } else {
        printk("Read error.\n");
    }
    if (regs->err_code & PAGE_USER) {
        printk("In user mode.\n");
    } else {
        printk("In kernel mode.\n");
    }
    // bit 3 为 1 表示错误是由保留位覆盖造成的
    if (regs->err_code & 0x8) {
        printk("Reserved bits being overwritten.\n");
    }
    // bit 4 为 1 表示错误发生在取指令的时候
    if (regs->err_code & 0x10) {
        printk("The fault occurred during an instruction fetch.\n");
    }

    map(addr, PAGE_PRESENT | PAGE_WRITE);
}
