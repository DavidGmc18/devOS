#pragma once

#include <page.h>
#include <stdint.h>

#define HHDM_BASE ((uintptr_t)0xFFFF888000000000)

typedef struct phys_addr_t* phys_addr;
typedef struct hhdm_addr_t* hhdm_addr;

static inline phys_addr hhdm_to_phys(hhdm_addr hhdm) {
    return (phys_addr)((uintptr_t)hhdm - HHDM_BASE);
}

static inline hhdm_addr phys_to_hhdm(phys_addr phys) {
    return (hhdm_addr)((uintptr_t)phys + HHDM_BASE);
}

hhdm_addr page_to_hhdm(struct page* page);
struct page* hhdm_to_page(hhdm_addr hhdm);