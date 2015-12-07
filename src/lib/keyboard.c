#include "idt.h"
#include "common.h"
#include "printk.h"
#include "types.h"

char keyboard[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', '\t', 
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 
    '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
    0,
    ' '

};

class KeyBoard {
public:
    char TypeIn() {
        uint8_t k = inb(0x60);
        if (IsShift(k)) {
            is_shifting = IsRelease(k) ? FALSE : TRUE;
        } else if (IsCtrl(k)) {
            is_ctrling = IsRelease(k) ? FALSE : TRUE;
        } else if (IsCapslock(k)) {
            if (IsPress(k))
                is_capslock = is_capslock ? FALSE : TRUE;
        } else if (IsRelease(k)) {

        } else {
            char res = keyboard[k];
            if (is_capslock)
                res += 'A' - 'a';
            if (is_shifting) {
                if ('a' <= res && res <= 'z') {
                    res += 'A' - 'a';
                } else {
                    res -= 'A' - 'a';
                }
            }
            return res;
        }
        return 0;
    }
    
private:
    uint8_t IsRelease(uint8_t k) const {
        return (k & 0x80);
    }

    uint8_t IsPress(uint8_t k) const {
        return !IsRelease(k);
    }

    uint8_t IsShift(uint8_t k) const {
        k &= 0x7F;
        return k == 42 || k == 54;
    }

    uint8_t IsCtrl(uint8_t k) const {
        k &= 0x7F;
        return k == 29;
    }
    
    uint8_t IsCapslock(uint8_t k) const {
        k &= 0x7F;
        return k == 58;
    }

    uint8_t is_shifting = FALSE;
    uint8_t is_ctrling = FALSE;
    uint8_t is_alting = FALSE;
    uint8_t is_capslock = FALSE;
};

void keyboard_isr(pt_regs *regs) {
    static KeyBoard keyboard;
    if (char ch = keyboard.TypeIn()) {
        printk("%c", ch);
    }
}

void init_keyboard() {
    register_isr(IRQ1, keyboard_isr);
}
