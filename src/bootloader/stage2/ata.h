#pragma once

#include <stdint.h>

#define ATA_ERRC_SUCCESS        0
#define ATA_ERRC_INVALID_BASE   1 // invalid bus
#define ATA_ERRC_SR_ERR         2
#define ATA_ERRC_TIMED_OUT      3
#define ATA_ERRC_FLOATING_BUS   4 // no disk present
#define ATA_ERRC_NOT_ATA_DISK   5

int ATA_io_delay(uint16_t bus);

int ATA_soft_reset(uint16_t bus);

int ATA_identify(uint16_t disk, void* buffer);

int ATA_read28(uint16_t disk, uint32_t LBA, uint8_t sectors, void* buffer);