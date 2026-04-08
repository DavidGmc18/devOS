#pragma once

// TODO is this good size for stack?
extern __attribute__((aligned(16))) unsigned char kern_stack[4096];
extern __attribute__((aligned(16))) unsigned char emergency_stack[4096];

#define __SET_STACK(s) __asm__ volatile("mov %0, %%rsp" :: "r"((s) + sizeof(s)) : "memory")

#ifdef DEBUG
// TODO this might be veru inacurate if due to branchles fn calls, look at it
// Paints the stack to estimate usage and help detecting stack underflows
#define __MARK_STACK(s, o) memset((s), 0xDF, sizeof(s)-(o));
#define __STACK_USED(s) ({          \
    int used = 0;                   \
    while(used < sizeof(s)) {       \
        if ((s)[used] != 0xDF) break; \
        used++;                     \
    }                               \
    sizeof(s) - used;               \
})
#endif