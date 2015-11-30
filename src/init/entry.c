#include "debug.h"

int kern_entry() {
    init_debug();
    console_clear();
    printk("%d %x %s\n", 123, 0x123F, "hello, code");
    panic("test");
    return 0;
}
