#include "timer.h"
#include "debug.h"
#include "common.h"
#include "idt.h"
#include "printk.h"

void timer_callback(pt_regs *regs) {
    static uint32_t tick = 0;
    printk("Tick: %d\n", tick++);
}

/**
 * param frequency: 中断频率
 */
void init_timer(uint32_t frequency) {
    // 注册timer
    register_isr(IRQ0, timer_callback);
    
    uint32_t divisor = 1193180 / frequency;
    
    // 设置芯片工作模式
    outb(0x43, 0x36);

    // 貌似这个频率只支持16位，所以最大65535, 这也限制了我们能够使用的最小中断频率，1193180/65525=18
    uint8_t low =(uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x40, low);
    outb(0x40, high);
}
