#ifndef TASK_H
#define TASK_H

#include "types.h"
#include "vmm.h"

enum TaskState {
    kTaskUninit = 0,
    kTaskSleeping = 1,
    kTaskRunnable = 2,
    kTaskZombie = 3
};

struct task_context {
    uint32_t esp;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;
};

typedef uint32_t pid_t;

struct task_struct {
    volatile TaskState state;
    pid_t pid;
    void* kstack;
    task_context context;
    task_struct* next;
};

#endif
