#pragma once

#include <stdint.h>

enum ATA_DEV {
    ATA_DEV_UNKNOWN,
    ATA_DEV_NONE,
    ATA_DEV_PATA,
    ATA_DEV_PATAPI
};

enum ATA_DEV ATA_identify(uint16_t id, uint16_t* identify_buf);

void ATA_test();