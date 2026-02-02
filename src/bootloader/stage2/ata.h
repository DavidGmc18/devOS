#pragma once

#include <stdint.h>

#define ATA_ERRC_SUCCESS        0
#define ATA_ERRC_INVALID_BASE   1 // invalid bus
#define ATA_ERRC_SR_ERR         2
#define ATA_ERRC_TIMED_OUT      3
#define ATA_ERRC_FLOATING_BUS   4 // no drive present
#define ATA_ERRC_NOT_ATA_DRIVE  5

int ATA_io_delay(uint16_t bus);

int ATA_soft_reset(uint16_t bus);

int ATA_identify(uint16_t drive, void* buffer);

int ATA_read28(uint16_t drive, uint32_t LBA, uint8_t sectors, void* buffer);
// int ATA_write28(uint16_t drive);



// #include <stdint.h>

// enum ATA_DEV {
//     ATA_DEV_UNKNOWN,
//     ATA_DEV_NONE,
//     ATA_DEV_PATA,
//     ATA_DEV_PATAPI
// };

// enum ATA_DEV ATA_identify(uint16_t id, uint16_t* identify_buffer);

// int ATA_update(uint16_t id);

// void ATA_test();