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
void kthread(kthread_entry fn);

/**
 * Call the scheduler to schedule kernel threads.
 */
void kthread_schedule();

void kthread_exit(task_struct*);

/**
 * Test
 */
static void thread1();

static void thread2();

void test_kthread();

#endif