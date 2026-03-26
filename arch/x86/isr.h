#pragma once

#include <stdint.h>

typedef struct {
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
} InterruptFrame;

void ISR_init();