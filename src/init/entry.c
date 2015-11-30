#include "console.h"

int kern_entry() {
    console_clear();
    console_write_hex(0x123F, rc_black, rc_green);
    console_write_dec(123, rc_black, rc_red);
    console_write_color("hello, code", rc_black, rc_green);
    return 0;
}
