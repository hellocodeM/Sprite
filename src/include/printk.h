#ifndef PRINTK_H
#define PRINTK_H

#include "types.h"
#include "string.h"
#include "console.h"

enum Format { kNone, kDec, kHex };

static void printk(char ch, Format f = kNone) { console_putc(ch); }

static void printk(const char* str, Format f = kNone) { console_write(str); }

static void printk(const uint8_t* str, Format f = kNone) { console_write((const char*)str); }

static void printk(int n, Format f = kNone) {
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

static void printk(uint32_t n, Format f = kNone) {
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
