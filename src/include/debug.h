#ifndef DEBUG_H
#define DEBUG_H

#include "console.h"
#include "string.h"

#define assert(x, info) \
    do { \
        if (!(x)) { \
            panic(info); \
        } \
    } while(0) 

#define static_assert(x) \
    switch (x) { case 0: case(x): ; }

void init_debug();

void panic(const char* msg);

void print_cur_status();

enum Format { kNone, kDec, kHex };

static void printk(char ch, Format f = kNone) { console_putc(ch); }

static void printk(const char* str, Format f = kNone) { console_write(str); }

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
