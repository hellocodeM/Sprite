#include "kthread.h"
#include "scheduler.h"
#include "new.hpp"

static TaskScheduler scheduler;

void init_kthread() {
    new (&scheduler) TaskScheduler();
}

pid_t kthread(kthread_entry fn) {
    return scheduler.submit(fn);
}

void kthread_schedule() {
    scheduler.schedule();
}

void kthread_exit(pid_t p) {
    scheduler.kill(p);
}

/*
void kthread_yield(pid_t p) {
    scheduler.yield(p);
}

void kthread_resume(pid_t p) {
    scheduler.resume(p);
}
*/
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
