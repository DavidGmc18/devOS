#pragma once

#include <stdint.h>
#include "gdt.h"

#define NO_IST 0
#define EMERG_IST 1

void tss_set(gdt_entry_t* tss_gdt_entry);