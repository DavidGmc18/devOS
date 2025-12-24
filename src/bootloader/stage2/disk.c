#include "disk.h"
#include "x86.h"

uint8_t DISK_Initialize(DISK* disk, uint8_t driveNumber) {
    uint8_t driveType;
    uint16_t cylinders, sectors, heads;

    uint8_t err = x86_Disk_GetDriveParams(disk->id, &driveType, &cylinders, &sectors, &heads);
    if (err) return err;

    disk->id = driveNumber;
    disk->cylinders = cylinders;
    disk->heads = heads;
    disk->sectors = sectors;

    return 0;
}

void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* headOut, uint16_t* sectorOut) {
    *sectorOut = lba % disk->sectors + 1;
    *headOut = (lba / disk->sectors) % disk->heads;
    *cylinderOut = (lba / disk->sectors) / disk->heads;
}

uint8_t DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, uint8_t* dataOut) {
    uint16_t cylinder, head, sector;

    DISK_LBA2CHS(disk, lba, &cylinder, &head, &sector);

    for (uint8_t i = 0; i < 3; i++) {
        uint8_t err = x86_Disk_Read(disk->id, cylinder, head, sector, sectors, dataOut);
        if (!err) return 0;

        x86_Disk_Reset(disk->id);
    }

    return 1;
}