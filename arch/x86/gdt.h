#pragma once

typedef enum {
    GDT_NULL_SEGMENT = 0x00,
    GDT_KERNEL_CODE_SEGMENT = 0x08,
    GDT_KERNEL_DATA_SEGMENT = 0x10
} GDT_Segments;

void GDT_init();