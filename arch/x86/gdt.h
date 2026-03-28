#pragma once

#include <stdint.h>

typedef enum {
    GDT_NULL_SEGMENT = 0x00,
    GDT_KERNEL_CODE_SEGMENT = 0x08,
    GDT_KERNEL_DATA_SEGMENT = 0x10,
    GDT_TSS_SEGMENT = 0x18, // 16B
} gdt_segment_t;

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_flags;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

void gdt_init();