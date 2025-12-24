#include "stdint.h"
#include "stdio.h"
#include "disk.h"

void _cdecl cstart_(uint16_t bootDrive) {
    DISK disk;

    if (DISK_Initialize(&disk, bootDrive)) {
        printf("Disk init error\r\n");
        return;
    }

    printf("Disk %i, C=%i H=%i S=%i\r\n", disk.id, disk.cylinders, disk.heads, disk.sectors);

    for (;;);
}