#include "kthread.h"
#include "scheduler.h"

static TaskScheduler scheduler;

void init_kthread() {
    scheduler.init();
}

void kthread(kthread_entry fn) {
    scheduler.submit(fn);
}

void kthread_schedule() {
    scheduler.schedule();
}

void kthread_exit(task_struct* task) {
    scheduler.exit(task);
}

static void thread1() {
    static int cnt = 0;
    for (int i = 0; i < 0x10000; i++) {
        if (++cnt ==0x1000) {
            printk("A");
            cnt = 0;
        }
    }
}

static void thread2() { 
    static int cnt = 0;
    for (int i = 0; i < 0x10000; i++) { 
        if (++cnt == 0x1000) {
            printk("B");
            cnt = 0;
        }
    }
}

void test_kthread() {
    kthread(&thread1);
    kthread(&thread2);
}
