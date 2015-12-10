#ifndef KTHREAD_H
#define KTHREAD_H

#include "printk.h"
#include "task.h"

typedef void (*kthread_entry)();

/**
 * Initialize kernel thread.
 */
void init_kthread();

/**
 * Create a kernel thread.
 */
pid_t kthread(kthread_entry fn);

/**
 * Call the scheduler to schedule kernel threads.
 */
void kthread_schedule();

/**
 * Normally exit or kill a thread
 */
void kthread_exit(pid_t);

/**
 * Yield a thread.
 */
//void kthread_yield(pid_t);

/**
 * Resume the thread
 */
//void kthread_resume(pid_t);

/**
 * Test
 */
static void thread1();

static void thread2();

void test_kthread();

#endif
