#include "printk.h"

int kern_entry() {
    console_clear();
    printk("%d %x %s", 123, 0x123F, "hello, code");
    return 0;
}
