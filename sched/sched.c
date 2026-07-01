#include "task.h"
#include <stddef.h>
#include <arch/x86/vmm.h>
#include <mm.h>
#include <arch/x86/gdt.h>

#define TASK_SLOTS 16
static struct task tasks[TASK_SLOTS];

static inline struct task* get_free_slot() {
    for (unsigned int i = 0; i < TASK_SLOTS; i++) {
        if (tasks[i].state == TASK_FREE) return &tasks[i];
    }
    return NULL;
}

static uint64_t next_pid = 0;

struct task* create_task(uint32_t priority) {
    struct task* task = get_free_slot();
    if (!task) return NULL;

    task->vmem = vmm_create_user_pml4();
    if (!task->vmem) return NULL;

    task->ctx.rflags = 0x202;
    task->ctx.cs = GDT_USER_CODE_SEGMENT | 3;
    task->ctx.ss = GDT_USER_DATA_SEGMENT | 3;

    task->priority = priority;

    task->pid = next_pid++;
    task->state = TASK_INIT;
    return task;
}

void run_task(struct task* task, uint64_t entry, uint64_t stack_top) {
    task->ctx.rip = (uint64_t)entry;
    task->ctx.rsp = (uint64_t)stack_top;
    task->state = TASK_RUNNING;
}

struct page* task_alloc_pages(struct task* task, unsigned char order) {
    struct page* page = alloc_pages(order);
    if (!page) return NULL;

    page->next = task->pages;
    page->prev = NULL;
    if (task->pages) task->pages->prev = page;
    task->pages = page;
    return page;
}

static struct task* current = NULL;

uint64_t clock = 0;
#include <printk.h>

void schedule(struct regs* r) {
    if (current) {
        current->ctx = *r;
        current->vtime += current->priority;
    }

    uint64_t min_vtime = UINT64_MAX;
    for (unsigned int i = 0; i < TASK_SLOTS; i++) {
        if (tasks[i].state != TASK_RUNNING || tasks[i].vtime >= min_vtime) continue;
        min_vtime = tasks[i].vtime;
        current = &tasks[i];
    }
    if (!current) return;

    for (unsigned int i = 0; i < TASK_SLOTS; i++) {
        if (tasks[i].state == TASK_RUNNING) tasks[i].vtime -= min_vtime;
    }

    clock++;
    if (clock >= 1024) {
        clock = 0;
        printk("A: rax=%lld    B: rax=%lld    C: rax=%lld    D: rax=%lld\n", tasks[0].ctx.rax, tasks[1].ctx.rax, tasks[2].ctx.rax, tasks[3].ctx.rax);
    }

    *r = current->ctx;
    vmm_set_table(current->vmem);
}