#include "disk.h"
#include "x86.h"

uint DISK_Initialize(DISK* disk, uint8_t driveNumber) {
    uint8_t driveType;
    uint16_t cylinders, heads, sectors;

    uint8_t err = x86_Disk_GetDriveParams(disk->id, &driveType, &cylinders, &heads, &sectors);
    if (err) return err;

    disk->id = driveNumber;
    disk->cylinders = cylinders + 1;
    disk->heads = heads + 1;
    disk->sectors = sectors;

    return 0;
}

void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* headOut, uint16_t* sectorOut) {
    *cylinderOut = (lba / disk->sectors) / disk->heads;
    *headOut = (lba / disk->sectors) % disk->heads;
    *sectorOut = lba % disk->sectors + 1;
}

uint DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, void far* dataOut) {
    uint16_t cylinder, head, sector;

    DISK_LBA2CHS(disk, lba, &cylinder, &head, &sector);

    uint err;
    for (uint8_t i = 0; i < 3; i++) {
        err = x86_Disk_Read(disk->id, cylinder, head, sector, sectors, dataOut);
        if (!err) return 0;

        x86_Disk_Reset(disk->id);
    }

    return err;
}