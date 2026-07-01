#pragma once

#include <stdint.h>
#include <arch/x86/interrupt.h>

enum task_state {
    TASK_FREE = 0,
    TASK_INIT,
    TASK_RUNNING,
    TASK_DEAD,
};

struct task {
    uint64_t pid;
    enum task_state state;

    // Context
    struct regs ctx;

    // Memory
    uint64_t* vmem; // PML4
    struct page* pages;

    // Scheduling
    uint64_t vtime;
    uint32_t priority;
};

struct task* create_task(uint32_t priority);
void run_task(struct task* task, uint64_t entry, uint64_t stack_top);

struct page* task_alloc_pages(struct task* task, unsigned char order);

// TODO kill & delete task