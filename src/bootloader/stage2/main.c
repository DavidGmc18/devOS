#include "stdint.h"
#include "stdio.h"
#include "disk.h"
#include "fat.h"

void _cdecl cstart_(uint16_t bootDrive) {
    DISK disk;

    if (DISK_Initialize(&disk, bootDrive)) {
        printf("Disk init error\r\n");
        return;
    }

    if (FAT_Initialize(&disk)) {
        printf("FAT init error\r\n");
        return;
    }

    FAT_File far* fd = FAT_Open(&disk, "/");
    FAT_DirectoryEntry entry;
    int i = 0;
    while (!FAT_ReadEntry(&disk, fd, &entry) && i++ < 8) {
        printf("  ");
        for (int i = 0; i < 11; i++)
            putc(entry.Name[i]);
        printf("\r\n");
    }
    FAT_Close(fd);

    char buffer[100];
    uint32_t read;
    FAT_File far* file = FAT_Open(&disk, "dir/test2.txt");
    while (read = FAT_Read(&disk, file, sizeof(buffer), buffer)) {
        for (uint32_t i = 0; i < read; i++) {
            if (buffer[i] == '\n') putc('\r');
            putc(buffer[i]);
        }
    }
    FAT_Close(file);
}
