#pragma once

#include <stdint.h>

struct regs {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp;
    uint64_t r8, r9, r10, r11, r12, r13, r4, r15;

    union {
        uint64_t vector_id;
        uint64_t syscall_id;
    };
    uint64_t error_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));