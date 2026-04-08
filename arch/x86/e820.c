#include "e820.h"
#include "stddef.h"
#include <string.h>
#include <printk.h>
#include <panic.h>

#define E820_MAX_ENTRIES 256

static struct e820_entry entries[E820_MAX_ENTRIES];
static struct e820_table e820_table;

/* * ============================================================================
 * BEGIN DERIVATIVE CODE 
 * The logic below is derived from the Linux kernel (arch/x86/kernel/e820.c).
 * This specific section is licensed under GPL-2.0-only.
 * SPDX-License-Identifier: GPL-2.0-only
 * ============================================================================
 */

struct change_point {
    uintptr_t addr;
    struct e820_entry* entry;
};

static struct change_point change_points[2*E820_MAX_ENTRIES];
// static struct e820_entry new_entries[E820_MAX_ENTRIES];
static struct e820_entry *overlap_list[E820_MAX_ENTRIES];

static inline int change_point_cmp(const struct change_point* a, const struct change_point* b) {
    if (a->addr != b->addr)
        return (a->addr > b->addr) ? 1 : -1;
    return (a->addr != a->entry->addr) - (b->addr != b->entry->addr);
}

static int e820_sanitize(struct e820_table* table, struct e820_table* new_table) {
    struct e820_entry *entries = table->entries;

    if (table->entries_count < 2)
        return 0;

    if (table->entries_count > E820_MAX_ENTRIES)
        return -1;

    uint32_t change_count = 0;
    for (int i = 0; i < table->entries_count; i++) {
        if (entries[i].addr + entries[i].size < entries[i].addr)
            return -1;

        if (entries[i].size > 0) {
            change_points[change_count].addr = entries[i].addr;
            change_points[change_count++].entry = &entries[i];
            change_points[change_count].addr = entries[i].addr + entries[i].size;
            change_points[change_count++].entry = &entries[i];
        }
    }

    // Sort
    for (int i = 1; i < change_count; i++) {
        struct change_point temp = change_points[i];
        int j = i - 1;
        while (j >= 0) {
            if (change_point_cmp(&change_points[j], &temp) < 0) break;
            change_points[j+1] = change_points[j];
            j--;
        }
        change_points[j + 1] = temp;
    }

    int overlap_entries = 0;	 /* Number of entries in the overlap table */
	int last_type = 0;		 /* Start with undefined memory type */
	uint64_t last_addr = 0;		 /* Start with 0 as last starting address */
    for (int chg_idx = 0; chg_idx < change_count; chg_idx++) {
        /* Keep track of all overlapping entries */
		if (change_points[chg_idx].addr == change_points[chg_idx].entry->addr) {
			/* Add map entry to overlap list (> 1 entry implies an overlap) */
			overlap_list[overlap_entries++] = change_points[chg_idx].entry;
		} else {
			/* Remove entry from list (order independent, so swap with last): */
			for (int idx = 0; idx < overlap_entries; idx++) {
				if (overlap_list[idx] == change_points[chg_idx].entry)
					overlap_list[idx] = overlap_list[overlap_entries-1];
			}
			overlap_entries--;
		}

        /*
		 * If there are overlapping entries, decide which
		 * "type" to use (larger value takes precedence --
		 * 1=usable, 2,3,4,4+=unusable)
		 */
		int current_type = 0;
		for (int idx = 0; idx < overlap_entries; idx++) {
			if (overlap_list[idx]->type > current_type)
				current_type = overlap_list[idx]->type;
		}

		/* Continue building up new map based on this information: */
		if (current_type != last_type) {
			if (last_type) {
				new_table->entries[new_table->entries_count].size = change_points[chg_idx].addr - last_addr;
				/* Move forward only if the new size was non-zero: */
				if (new_table->entries[new_table->entries_count].size != 0)
					/* No more space left for new entries? */
					if (++new_table->entries_count >= E820_MAX_ENTRIES)
						break;
			}
			if (current_type) {
				new_table->entries[new_table->entries_count].addr = change_points[chg_idx].addr;
				new_table->entries[new_table->entries_count].type = current_type;
				last_addr = change_points[chg_idx].addr;
			}
			last_type = current_type;
		}
    }

    for (int i = 0; i < new_table->entries_count; i++) {
        if (new_table->entries[i].addr % 0x1000 || new_table->entries[i].size % 0x1000) {
            printk(KERN_WARNING "[WARN] Unaligned E820 entry: base=%#llx size=%#llx\n", new_table->entries[i].addr, new_table->entries[i].size); 
        }
    }

    return 0;
}

/* * ============================================================================
 * END DERIVATIVE CODE
 * ============================================================================
 */

void e820_init(struct e820_table* table) {
    e820_table.entries = entries;

    if (e820_sanitize(table, &e820_table)) {
        panic("Failed to sanitize E820 memory map!\n");
    }

    printk("[OK] E820 sanitized & copied\n");
}

struct e820_table* e820_get_table() {
    return &e820_table;
}