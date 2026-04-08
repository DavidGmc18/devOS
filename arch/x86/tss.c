#include "tss.h"
#include "GDT_MACROS.h"

typedef struct {
    uint32_t reserved0;
    uint64_t rsp[3];
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb;
} __attribute__((packed)) tss_t;

// TODO is this good size for stack?
static __attribute__((aligned(16))) unsigned char rsp0_stack[4096];
static __attribute__((aligned(16))) unsigned char emergency_stack[4096];

static tss_t tss = {
    .rsp[0] = (uintptr_t)rsp0_stack + sizeof(rsp0_stack),
    .ist[EMERG_IST-1] = (uintptr_t)emergency_stack + sizeof(emergency_stack),
    .iopb = sizeof(tss_t)
};

void tss_set(gdt_entry_t* tss_gdt_entry) {
    tss_gdt_entry->limit_low = sizeof(tss_t) - 1;
    tss_gdt_entry->base_low = (uint16_t)(uintptr_t)&tss;
    tss_gdt_entry->base_mid = (uint8_t)((uintptr_t)&tss >> 16);
    tss_gdt_entry->access = P(1) | DPL(0) | S(0) | E(1) | DC(0) | RW(0) | A(1);
    tss_gdt_entry->limit_flags = 0x0;
    tss_gdt_entry->base_high = (uint8_t)((uintptr_t)&tss >> 24);

    uint32_t* tss_high = (uint32_t*)(tss_gdt_entry+1);
    tss_high[0] = (uint32_t)((uintptr_t)&tss >> 32);
    tss_high[1] = 0;
}