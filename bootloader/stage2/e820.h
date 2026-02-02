#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t Base;
    uint64_t Length;
    uint32_t Type;
    uint32_t ACPI;
} E820_MemoryBlock;

enum E820_MemoryBlock {
    E820_USABLE = 1,
    E820_RESERVED = 2,
    E820_ACPI_RECLAIMABLE = 3,
    E820_ACPI_NVS = 4,
    E820_BAD_MEMORY = 5,
};

int __attribute__((cdecl)) i686_E820GetNextBlock(E820_MemoryBlock* block, uint32_t* continuationId);