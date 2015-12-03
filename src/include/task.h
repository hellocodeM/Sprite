#ifndef TASK_H
#define TASK_H

#include "types.h"
#include "pmm.h"
#include "vmm.h"

enum task_state {
    TASK_UNINIT = 0,
    TASK_SLEEPING = 1,
    TASK_RUNNABLE = 2,
    TASK_ZONBIE = 3
};

struct context {
    uint32_t esp, ebp, ebx, esi, edi, eflags;
};

struct mm_struct {
    pgd_t *pgd_dir;
};

struct task_struct {
    volatile task_state state;
    pid_t pid;
    void *stack;
    mm_struct *mm;
    struct context context;
    task_struct *next;
};

extern pid_t now_pid;

int32_t kernel_thread(int (*fn)(void*), void *arg);

void kthread_exit();

#endif
