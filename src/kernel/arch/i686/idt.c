#include "idt.h"
#include <stdint.h>
#include <util/binary.h>

typedef struct {
    uint16_t BaseLow;
    uint16_t SegmentSelector;
    uint8_t Reserved;
    uint8_t Flags;
    uint16_t BaseHigh;
} __attribute__((packed)) IDT_Entry;

typedef struct {
    uint16_t Limit;
    IDT_Entry* Ptr;
} __attribute__((packed)) IDT_Descriptor;

IDT_Entry g_IDT[256];

IDT_Descriptor g_IDT_Descriptor = {sizeof(g_IDT) - 1, g_IDT};

void __attribute__((cdecl)) i686_IDT_Load(IDT_Descriptor* descriptor);

void i686_IDT_EnableGate(uint8_t interrupt) {
    FLAG_SET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void i686_IDT_DisabkeGate(uint8_t interrupt) {
    FLAG_UNSET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void i686_IDT_Initialize() {
    i686_IDT_Load(&g_IDT_Descriptor);
}

void i686_IDT_SetGate(uint8_t interrupt, void* base, uint16_t segmentDescriptor, uint8_t flags) {
    g_IDT[interrupt].BaseLow = ((uint32_t)base) & 0xFFFF;
    g_IDT[interrupt].SegmentSelector = segmentDescriptor;
    g_IDT[interrupt].Reserved = 0;
    g_IDT[interrupt].Flags = flags;
    g_IDT[interrupt].BaseHigh = ((uint32_t)base >> 16) & 0xFFFF;
}