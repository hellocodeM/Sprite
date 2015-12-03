#include "idt.h"
#include "common.h"
#include "printk.h"

char keyboard[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', '\t', 
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 
    '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*'
};


void keyboard_isr(pt_regs *regs) {
    uint8_t code = inb(0x60);
    if (code & 0x80) {
        
    } else {
        printk("%c", keyboard[code]);
    }
}

void init_keyboard() {
    register_isr(IRQ1, keyboard_isr);
}
