#pragma once

#include <stdint.h>
#include <driver/ata/ata.h>

typedef struct {
    uint64_t start;
    uint64_t length;
    uint32_t type;
    uint32_t ACPI;
} MemoryRegion;

typedef struct {
    uint32_t region_count;
    MemoryRegion* regions;
} MemoryInfo;

typedef struct {
    ATA_drive_t drive;
    uint8_t partition;
} BootDevice;

typedef struct {
    BootDevice boot_device;
    MemoryInfo memory_info;
} BootParams;
