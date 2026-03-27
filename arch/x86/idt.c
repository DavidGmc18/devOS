#include "idt.h"
#include <string.h>
#include <kernel/panic.h>

typedef struct {
    uint16_t offset_low;
    uint16_t segment;
    uint8_t ist : 3, zero : 5;
    uint8_t type : 5, dpl : 2, p : 1;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed)) IDT_Gate;

typedef struct {
    uint16_t size;
    IDT_Gate* base;
} __attribute__((packed)) IDT_Descriptor;

static IDT_Gate gates[256];
static IDT_Descriptor descriptor = {sizeof(gates) - 1, gates};

void IDT_init() {
    memset(gates, 0, sizeof(gates));
    __asm__ volatile ("lidt %0" :: "m"(descriptor) : "memory");
}

void IDT_set_gate(uint8_t interrupt, void* offset, uint16_t segment, uint8_t ist, uint8_t type, uint8_t dpl, bool p) {
    if (!offset && p) panic("Attempted to set IDT %#X to NULL while enabled!\n", interrupt);
    if (segment == 0x0 || segment % 8 != 0) panic("Attempted to set IDT %#X segment to %#X!\n", interrupt, segment);
    
    gates[interrupt].offset_low = (uintptr_t)offset;
    gates[interrupt].segment = segment;
    gates[interrupt].ist = ist;
    gates[interrupt].zero = 0;
    gates[interrupt].type = type;
    gates[interrupt].dpl = dpl;
    gates[interrupt].p = p;
    gates[interrupt].offset_mid = ((uintptr_t)offset) >> 16;
    gates[interrupt].offset_high = ((uintptr_t)offset) >> 32;
    gates[interrupt].reserved = 0;
}