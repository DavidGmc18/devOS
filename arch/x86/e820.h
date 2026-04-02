#pragma once

#include <stdint.h>

enum e820_type {
	E820_TYPE_RAM = 1,
	E820_TYPE_RESERVED = 2,
	E820_TYPE_ACPI = 3,
	E820_TYPE_NVS = 4,
	E820_TYPE_UNUSABLE = 5
};

struct e820_entry {
	uint64_t addr;
	uint64_t size;
	enum e820_type type;
} __attribute__((packed)) ;

struct e820_table {
	uint32_t entries_count;
	uint32_t _reserved; // Explicit padding for 32/64bit ABI compatibility
	// Union ensures the ptr is always 64bit wide. 
    // This allows a 32bit bootloader to pass a pointer to 64bit kernel
	union {
        struct e820_entry* entries;
        uint64_t entries_64;
    };
} __attribute__((packed));