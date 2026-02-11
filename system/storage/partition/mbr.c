#include "mbr.h"
#include "memory.h"

#define MBR_TABLE 446

int MBR_get_table(ATA_drive_t drive, MBR_Table* table) {
    uint8_t buffer[512];

    int error = ATA_read28(drive, 0, 1, buffer);
    if (error != ATA_ERRC_SUCCESS) {
        return error;
    }

    memcpy(table, buffer + MBR_TABLE, sizeof(MBR_Table));

    return MBR_ERRC_SUCCESS;
}