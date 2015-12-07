#ifndef CONSOLE_H
#define CONSOLE_H

#include "types.h"

typedef enum real_color {
    rc_black = 0,
    rc_blue = 1,
    rc_green = 2,
    rc_cyan = 3,
    rc_red = 4,
    rc_magenta = 5,
    rc_brown = 6, 
    rc_light_grey = 7,
    rc_dark_grey = 8,
    rc_light_blue = 9,
    rc_light_green = 10,
    rc_light_cyan = 11,
    rc_light_red = 12,
    rc_light_magenta = 13,
    rc_light_brown = 14,
    rc_white = 15
} real_color_t;

// 清屏
void console_clear();

// 输出一个字符
void console_putc(char c, real_color_t back = rc_black, real_color_t fore = rc_white);

// 输出一个字符串，带颜色
void console_write(const char *cstr, real_color_t back = rc_black, real_color_t fore = rc_white);

// 输出一个十六进制数
void console_write_hex(uint32_t n, real_color_t back = rc_black, real_color_t fore = rc_white);

// 输出一个十进制整数
void console_write_dec(uint32_t n, real_color_t back = rc_black, real_color_t fore = rc_white);

#endif
