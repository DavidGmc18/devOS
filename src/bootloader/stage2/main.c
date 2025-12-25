#include "stdint.h"
#include "stdio.h"
#include "disk.h"

void _cdecl cstart_(uint16_t bootDrive) {
    DISK disk;

    if (DISK_Initialize(&disk, bootDrive)) {
        printf("Disk init error\r\n");
        return;
    }

    printf("Disk %i: C=%i H=%i S=%i\r\n", disk.id, disk.cylinders, disk.heads, disk.sectors);

    if (DISK_ReadSectors(&disk, 19, 1, (void far*)0)) {
        printf("Disk read error\r\n");
        return;
    }

    for (;;);
}