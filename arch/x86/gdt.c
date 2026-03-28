#include "gdt.h"
#include "tss.h"
#include "GDT_MACROS.h"
#include <printk.h>

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
    },

    [GDT_TSS_SEGMENT/8] = {0},
    [GDT_TSS_SEGMENT/8+1] = {0},
};

static struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdtr;

void GDT_init() {
    TSS_set(gdt + GDT_TSS_SEGMENT/8);

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint64_t)&gdt;

    __asm__ volatile(
        "lgdt %[gdtr] \n"
        "pushq %[code_seg] \n"
        "leaq  1f(%%rip), %%rax \n"
        "pushq %%rax \n"
        "lretq \n"
        "1: \n"

        // Data segment
        "mov %[data_seg], %%rax \n"
        "mov %%ax, %%ds \n"
        "mov %%ax, %%es \n"
        "mov %%ax, %%fs \n"
        "mov %%ax, %%gs \n"
        "mov %%ax, %%ss \n"

        // TSS segment
        "mov %[tss_seg], %%ax \n"
        "ltr %%ax \n"
        ::
        [gdtr] "m"(gdtr),
        [code_seg] "i"(GDT_KERNEL_CODE_SEGMENT),
        [data_seg] "i"(GDT_KERNEL_DATA_SEGMENT),
        [tss_seg] "i"(GDT_TSS_SEGMENT)
        : "rax", "memory"
    );

    printk("[OK] GDT and TSS initialized\n");
}