#pragma once

#include <arch/x86/io.h>

// TODO is this good size for stack?
extern __attribute__((aligned(16))) unsigned char kern_stack[4096];
extern __attribute__((aligned(16))) unsigned char emergency_stack[4096];

#define __SET_STACK(s) set_rsp((s) + sizeof(s));

// TODO this might be very inaccurate due to branchless fn calls, look at it
// Paints the stack to estimate usage and help detecting stack underflows
#define __MARK_STACK(s, o) memset((s), 0xDF, sizeof(s)-(o));
#define __STACK_USED(s) ({              \
    int used = 0;                       \
    while(used < sizeof(s)) {           \
        if ((s)[used] != 0xDF) break;   \
        used++;                         \
    }                                   \
    sizeof(s) - used;                   \
})
