#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"

extern task_struct *running_proc_head;
extern task_struct *wait_proc_head;
extern task_struct *current;

void init_sched();

void schedule();

void change_task_to(task_struct *next);

void switch_to(context *prev, context *next);

#endif
