#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"
#include "common.h"
#include "kmalloc.h"

/**
 * context swap
 */
extern "C" {
void switch_to(task_context* last, task_context* target);
}

class TaskScheduler {
public:
    void init() {
        pid_cnt_ = 0;
        runnable_tasks_ = nullptr;

        extern char* kern_stack;  // define in lib/entry.c
        running_ = reinterpret_cast<task_struct*>(&kern_stack);
        running_->state = kTaskRunnable;
        running_->pid = pid_cnt_++;
        running_->kstack = running_;
        running_->next = running_;

        runnable_tasks_ = running_;
    }

    pid_t submit(kthread_entry fn) {
        task_struct* new_task = reinterpret_cast<task_struct*>(kmalloc(STACK_SIZE));
        assert(new_task != nullptr, "Scheduler submit error");

        bzero(new_task, sizeof(task_struct));

        // use the stack topest to store the task struct
        new_task->state = kTaskRunnable;
        new_task->kstack = running_;
        new_task->pid = pid_cnt_++;

        // use stack from the bottom
        extern void kthread_exit(task_struct*); // define in kthread.h
        uint32_t* stack_top = reinterpret_cast<uint32_t*>((uint32_t)new_task + STACK_SIZE);
        *(--stack_top) = (uint32_t)new_task; // the parameter to kthread_exit
        *(--stack_top) = 0;                 // return address above the kthread_exit frame
        *(--stack_top) = (uint32_t)kthread_exit;
        *(--stack_top) = (uint32_t)fn;

        new_task->context.esp = reinterpret_cast<uint32_t>(stack_top);
        new_task->context.eflags = 0x200;

        // push_back the new task
        new_task->next = runnable_tasks_;
        task_struct* cur = runnable_tasks_;
        while (cur->next != runnable_tasks_) {
            cur = cur->next;
        }
        cur->next = new_task;

        return new_task->pid;
    }

    void schedule() {
        if (running_) {
            change_task_to(running_->next);
        }
    }

    /**
     * kill a task
     */
    void exit(task_struct* task) {
        pid_t pid = task->pid;
        // just 1 thread
        if (runnable_tasks_->next == runnable_tasks_) {
            runnable_tasks_->next = nullptr;
            running_ = nullptr;
        } else {
        // more than one thread
            task_struct *prev, *cur;
            prev = runnable_tasks_;
            cur = runnable_tasks_->next;
            while (cur != task) {
                cur = cur->next;
                prev = prev->next;
            }
            prev->next = cur->next;
        }
        kfree(task);
        printk("thread %d exit\n", pid);
    }

private:
    void change_task_to(task_struct* target) {
        if (running_ != target) {
            task_struct* last = running_;
            running_ = target;
            switch_to(&(last->context), &(running_->context));
        }
    }

    /* data members */
    task_struct* runnable_tasks_;
    task_struct* running_;
    pid_t pid_cnt_;
};

#endif
