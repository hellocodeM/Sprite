#include "console.h"
#include "common.h"
#include "vmm.h"

// 显存地址
static uint16_t *video_memory = (uint16_t*)(0xB8000 + PAGE_OFFSET);

// 光标地址
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

static void move_cursor() {
    uint16_t cursor_loc = cursor_y * 80 + cursor_x;
    
    outb(0x3D4, 14);                // 设置高字节
    outb(0x3D5, cursor_loc >> 8);
    outb(0x3D4, 15);                // 设置低字节
    outb(0x3D5, cursor_loc);
}

// 用背景色和前景色来构造一个颜色
inline static uint8_t make_color(real_color_t back, real_color_t fore) {
    return ((uint8_t)back << 4) | (0x0F & (uint8_t)fore);
}

inline static uint16_t make_char(char ch, uint8_t color) {
    return (((uint16_t)color) << 8) | ((uint16_t)ch);
}

void console_clear() {
    uint8_t color = make_color(rc_black, rc_white);
    uint16_t blank = make_char(' ', color);
    
    for (int i = 0; i < 80 * 25; i++) {
        video_memory[i] = blank;
    }
    
    cursor_x = 0;
    cursor_y = 0;
    move_cursor();
}

static void scroll() {
    uint16_t blank = make_char(' ', make_color(rc_black, rc_white));
    if (cursor_y >= 25) {
        for (int i = 0 * 80; i < 24 * 80; i++) {
            video_memory[i] = video_memory[i+80];
        }
        for (int i = 24 * 80; i < 25 * 80; i++) {
            video_memory[i] = blank;
        }
        cursor_y = 24;
    }
}

void console_putc(char c, real_color_t back, real_color_t fore) {
    uint16_t ch = make_char(c, make_color(back, fore));

    switch (c) {
        case '\b':
            if (cursor_x)
                --cursor_x;
            break;
        case '\t':
            cursor_x = (cursor_x+8) & ~(0b111);
            break;
        case '\r':
            cursor_x = 0;
            break;
        case '\n':
            cursor_x = 0;
            ++cursor_y;
            break;
        default:
            if (c >= ' ') {
                video_memory[cursor_y*80+cursor_x] = ch;
                ++cursor_x;
            }
    }
    if (cursor_x >= 80) {
        cursor_x = 0;
        ++cursor_y;
    }
    scroll();
    move_cursor();
}

void console_write(const char *str, real_color_t back, real_color_t fore) {
    while (*str) {
        console_putc(*str++, back, fore);
    }
}

// 输出单个十六进制字符
inline static void console_write_hex_single(uint32_t n, real_color_t back, real_color_t fore) {
    if (n < 10)
        console_putc('0' + n, back, fore);
    else
        console_putc('A' + n - 10, back, fore);
}

void console_write_hex(uint32_t n, real_color_t back, real_color_t fore) {
    const uint8_t mask = 0xF;
    uint8_t started = FALSE;
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t slice = (n & (mask << i)) >> i; 
        if (!started && slice != 0) {
            console_write_hex_single(slice, back, fore);
            started = TRUE;
        } else if (started) {
            console_write_hex_single(slice, back, fore);
        }
    }
    if (!started)
        console_write_hex_single(0, back, fore);
}

void console_write_dec(uint32_t n, real_color_t back, real_color_t fore) {
    if (n < 10) {
        console_putc('0' + n, back, fore);
    } else {
        console_write_dec(n / 10, back, fore);
        console_putc('0' + n % 10, back, fore);
    }
}
