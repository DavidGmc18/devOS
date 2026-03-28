#include "gdt.h"

#include <stdint.h>

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_flags;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

#define A(v) (v&1)
#define RW(v) ((v&1) << 1)
#define DC(v) ((v&1) << 2)
#define E(v) ((v&1) << 3)
#define S(v) ((v&1) << 4)
#define DPL(v) ((v&3) << 5)
#define P(v) ((v&1) << 7)

#define LIMIT_HIGH(v) (v&0xF)
#define L(v) ((v&1) << 5)
#define DB(v) ((v&1) << 6)
#define G(v) ((v&1) << 7)

static gdt_entry_t gdt[] = {
    [GDT_NULL_SEGMENT/8] = {0},

    [GDT_KERNEL_CODE_SEGMENT/8] = {
        .limit_low=0xFFFF,
        .access = A(0) | RW(1) | DC(0) | E(1) | S(1) | DPL(0) | P(1),
        .limit_flags = LIMIT_HIGH(0xF) | L(1) | DB(0) | G(1) 
    },

    [GDT_KERNEL_DATA_SEGMENT/8] = {
        .limit_low=0xFFFF,
        .access = A(0) | RW(1) | DC(0) | E(0) | S(1) | DPL(0) | P(1),
        .limit_flags = LIMIT_HIGH(0xF) | L(0) | DB(1) | G(1) 
    }
};

static struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdtr;

void GDT_init() {
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint64_t)&gdt;

    __asm__ volatile(
        "lgdt %[gdtr] \n"
        "pushq %[code_seg] \n"
        "leaq  1f(%%rip), %%rax \n"
        "pushq %%rax \n"
        "lretq \n"
        "1: \n"
        "mov %[data_seg], %%rax \n"
        "mov %%ax, %%ds \n"
        "mov %%ax, %%es \n"
        "mov %%ax, %%fs \n"
        "mov %%ax, %%gs \n"
        "mov %%ax, %%ss \n"
        ::
        [gdtr] "m"(gdtr),
        [code_seg] "i"(GDT_KERNEL_CODE_SEGMENT),
        [data_seg] "i"(GDT_KERNEL_DATA_SEGMENT)
        : "rax", "memory"
    );
}