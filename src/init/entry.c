#include "printk.h"
#include "debug.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"

void fuck(pt_regs *regs) {
    printk("holy, shit\n");
}

extern "C" int kern_entry() {
    init_debug();
    init_gdt();
    init_idt();

    register_isr(255, fuck);
    init_timer(20);
    
    console_clear();
    printk("Hello, Code\n");

    asm volatile ("sti");
    asm volatile ("int $0xFF");
    return 0;
}
