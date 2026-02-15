#pragma once

#include <driver/ata/ata.h>

int FAT_initialize(ATA_drive_t boot_drive, uint32_t sector_offset);

int FAT_load_file(const char* path, void* buffer);