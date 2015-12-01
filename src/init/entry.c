#include "printk.h"
#include "debug.h"
#include "gdt.h"

int kern_entry() {
    init_debug();
    init_gdt();
    console_clear();
    printk("%d %x %s\n", 123, 0x123F, "hello, code");
    return 0;
}
