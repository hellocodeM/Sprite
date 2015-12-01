#include "printk.h"
#include "debug.h"
#include "gdt.h"
#include "idt.h"

void fuck(pt_regs *regs) {
    printk("holy, shit\n");
}

extern "C" int kern_entry() {
    init_debug();
    init_gdt();
    init_idt();
    register_isr(255, fuck);
    console_clear();
    printk("%d %x %s\n", 123, 0x123F, "hello, code");

    asm volatile ("int $0x3");
    asm volatile ("int $0x4");
    asm volatile ("int $0xFF");
    return 0;
}
