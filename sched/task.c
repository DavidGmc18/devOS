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

static uint64_t min_vtime = 0;

void run_task(struct task* task, uint64_t entry, uint64_t stack_top) {
    task->ctx.rip = (uint64_t)entry;
    task->ctx.rsp = (uint64_t)stack_top;
    task->vtime = min_vtime;
    task->state = TASK_READY;
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

// int kill_task(uint64_t pid) {
//     struct task* task = NULL;
//     for (unsigned int i = 0; ; i++) {
//         if (tasks[i].pid != pid) continue;
//         task = &tasks[i];
//         break;
//     }
//     if (!task) return -1;
//     // task->state = TASK_DEAD;
//     return 0;
// }