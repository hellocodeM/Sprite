#include "printk.h"
#include "debug.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "pmm.h"

void fuck(pt_regs *regs) {
    printk("holy, shit\n");
}

void show_kern_mmap() {
    printk("kernel in memory start: 0x%s\n", kern_start);
    printk("kernel in memory end: 0x%s\n", kern_end);
}

extern "C" int kern_entry() {
    init_debug();
    init_gdt();
    init_idt();
    init_pmm();

    register_isr(255, fuck);
    init_timer(20);
    
    console_clear();
    printk("Hello, Code\n");

    show_mmap();
    uint32_t alloc_addr = NULL;
    printk("Test physical memory alloc: \n");

    alloc_addr = pmm_alloc_page();
    printk("alloc physical addr: 0x%x\n", alloc_addr);
    alloc_addr = pmm_alloc_page();
    printk("alloc physical addr: 0x%x\n", alloc_addr);
    alloc_addr = pmm_alloc_page();
    printk("alloc physical addr: 0x%x\n", alloc_addr);
    
    return 0;
}
