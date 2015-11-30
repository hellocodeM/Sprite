#ifndef PRINTK_H
#define PRINTK_H

#include "console.h"
#include "string.h"

enum Format { kNone, kDec, kHex };

void printk(char ch, Format f = kNone) { console_putc(ch); }

void printk(const char* str, Format f = kNone) { console_write(str); }

void printk(int n, Format f) {
    switch (f) {
        case kDec:
            console_write_dec(n);
            break;
        case kHex:
            console_write_hex(n);
            break;
        default:
            console_write_dec(n);
    }
}

template <class Head, class... Tail>
void printk(const char* format, Head head, Tail... tail) {
    const char* pos = strfind(format, '%');
    while (format != pos) console_putc(*format++);

    if (pos[0] && pos[1]) {
        switch (pos[1]) {
            case 'd':
                printk(head, kDec);
                break;
            case 'x':
                printk(head, kHex);
                break;
            case 's':
                printk(head);
                break;
            default:
                printk('%');
                printk(pos[1]);
        }
        printk(pos + 2, tail...);
    }
}

#endif
