#pragma once

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %1, %0" :: "Nd"(port), "a"(value));
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %1, %0" :: "Nd"(port), "a"(value));
}

static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %1, %0" :: "Nd"(port), "a"(value));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait() {
    outb(0x80, 0);
}

static inline void halt() {
    __asm__ volatile ("hlt" ::: "memory");
}

static inline void cli() {
    __asm__ volatile ("cli" ::: "memory");
}

static inline void sti() {
    __asm__ volatile ("sti" ::: "memory");
}

typedef struct {
    uint64_t cf : 1;    // 0: Carry Flag
    uint64_t res1 : 1;  // 1: Reserved (Always 1 in hardware)
    uint64_t pf : 1;    // 2: Parity Flag
    uint64_t res2 : 1;  // 3: Reserved
    uint64_t af : 1;    // 4: Auxiliary Carry Flag
    uint64_t res3 : 1;  // 5: Reserved
    uint64_t zf : 1;    // 6: Zero Flag
    uint64_t sf : 1;    // 7: Sign Flag
    uint64_t tf : 1;    // 8: Trap Flag (Single step)
    uint64_t intf : 1;  // 9: Interrupt Flag (IF)
    uint64_t df : 1;    // 10: Direction Flag
    uint64_t of : 1;    // 11: Overflow Flag
    uint64_t iopl : 2;  // 12-13: I/O Privilege Level
    uint64_t nt : 1;    // 14: Nested Task
    uint64_t res4 : 1;  // 15: Reserved
    uint64_t rf : 1;    // 16: Resume Flag
    uint64_t vm : 1;    // 17: Virtual 8086 Mode
    uint64_t ac : 1;    // 18: Alignment Check
    uint64_t vif: 1;    // 19: Virtual Interrupt Flag
    uint64_t vip : 1;   // 20: Virtual Interrupt Pending
    uint64_t id : 1;    // 21: ID Flag (CPU identification support)
    uint64_t reserved : 42;
} __attribute__((packed)) rflags_t;

static inline rflags_t get_rflags() {
    rflags_t rflags;
    __asm__ volatile (
        "pushfq\n\t"
        "popq %0"
        : "=rm" (rflags)
        :: "memory"
    );
    return rflags;
}