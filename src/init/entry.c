#include "printk.h"
#include "debug.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "pmm.h"
#include "vmm.h"
#include "keyboard.h"

void kern_init();

multiboot_t *glb_mboot_ptr;

char kern_stack[STACK_SIZE];

// 临时页表
__attribute__((section(".init.data"))) pde_t *pde_tmp = (pde_t *) 0x1000;
__attribute__((section(".init.data"))) pde_t *pte_low = (pde_t *) 0x2000;
__attribute__((section(".init.data"))) pde_t *pte_high = (pde_t *) 0x3000;

extern "C" __attribute__((section(".init.text"))) void kern_entry() {
    pde_tmp[0] = (uint32_t) pte_low | PAGE_PRESENT | PAGE_WRITE;
    pde_tmp[PGD_INDEX(PAGE_OFFSET)] = (uint32_t)pte_high | PAGE_PRESENT | PAGE_WRITE;

    for (int i = 0; i < 1024; i++) {
        // 内核虚拟地址前4MB一对一映射到物理地址前4MB
        // 内核虚拟地址（0xC0000000~0xC0400000）映射到物理地址前4MB
        pte_high[i] = pte_low[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
    }

    // 设置页目录
    asm volatile ("mov %0, %%cr3" : : "r"(pde_tmp));
    
    // 启用分页, 把cr0寄存器的paging位设置为1
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile ("mov %0, %%cr0" : : "r"(cr0));

    // 使用新的内核栈
    uint32_t kern_stack_top = ((uint32_t)kern_stack + STACK_SIZE) & 0xFFFFFFF0;
    asm volatile ("mov %0, %%esp;"
            "mov $0, %%ebp;" : : "r"(kern_stack_top));

    glb_mboot_ptr = mboot_ptr_tmp + PAGE_OFFSET;
    
    kern_init();
}


void show_kern_mmap() {
    printk("kernel in memory start: 0x%s\n", kern_start);
    printk("kernel in memory end: 0x%s\n", kern_end);
}

void kern_init() {
    init_debug();
    init_gdt();
    init_idt();
    init_pmm();
    init_keyboard();
    //init_vmm();
    //init_timer(20);
    
    console_clear();
    printk("Hello, Code\n");

    while (1)
        ;
}
