#include "printk.h"
#include "debug.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "vmm.h"
#include "pmm.h"
#include "keyboard.h"
#include "kmalloc.h"
#include "kthread.h"
#include "ide.h"
#include "common.h"
#include "new.h"
#include "atomic.h"
#include "fs.h"

void kern_init();

multiboot_t *glb_mboot_ptr;

char kern_stack[STACK_SIZE];

// 临时页表
__attribute__((section(".init.data"))) pde_t *pde_tmp = (pde_t *)0x1000;
__attribute__((section(".init.data"))) pde_t *pte_low = (pde_t *)0x2000;
__attribute__((section(".init.data"))) pde_t *pte_high = (pde_t *)0x3000;

extern "C" __attribute__((section(".init.text"))) void kern_entry() {
    pde_tmp[0] = (uint32_t)pte_low | PAGE_PRESENT | PAGE_WRITE;
    pde_tmp[PGD_INDEX(PAGE_OFFSET)] = (uint32_t)pte_high | PAGE_PRESENT | PAGE_WRITE;

    for (int i = 0; i < 1024; i++) {
        // 内核虚拟地址前4MB一对一映射到物理地址前4MB
        // 内核虚拟地址（0xC0000000~0xC0400000）映射到物理地址前4MB
        pte_high[i] = pte_low[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
    }

    // set page directory
    asm volatile("mov %0, %%cr3;" : : "r"(pde_tmp));

    // start paging
    uint32_t cr0;
    asm volatile("mov %%cr0, %0;" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0;" : : "r"(cr0));

    // 使用新的内核栈
    uint32_t kern_stack_top = ((uint32_t)kern_stack + STACK_SIZE) & 0xFFFFFFF0;
    asm volatile(
        "mov %0, %%esp;"
        "mov $0, %%ebp;"
        :
        : "r"(kern_stack_top));

    glb_mboot_ptr = mboot_ptr_tmp + PAGE_OFFSET;

    kern_init();
}

void kern_init() {
    init_debug();
    init_gdt();
    init_idt();
    init_pmm();
    init_keyboard();
    init_vmm();
    init_heap();
    init_kthread();
    init_timer(200);
    assert(ide_init() == 0, "ide not available");
    init_fs();

    console_clear();
    printk("Hello, Code\n");

    // show_kern_mmap();
    // test_vmm();
    // test_kthread();
    // test_ide();
    test_fs();

    while (1) cpu_hlt();
}
