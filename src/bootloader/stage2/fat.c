#include "fat.h"
#include "stdio.h"
#include "memdefs.h"
#include "utility.h"

#define SECTOR_SIZE             512

#pragma pack(push, 1)
typedef struct {
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;
    uint8_t VolumeLabel[11];
    uint8_t SystemId[8];
} FAT_BootSector;
#pragma pack(pop)

typedef struct
{
    uint8_t Buffer[SECTOR_SIZE];
    FAT_File Public;
    bool Opened;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;

} FAT_FileData;

typedef struct {
    union {
        FAT_BootSector BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;
} FAT_Data;

static FAT_Data far* g_Data;
static uint8_t far* g_Fat = NULL;
static FAT_DirectoryEntry far* g_RootDirectory = NULL;
static uint32_t g_RootDirectoryEnd;

uint FAT_ReadBootSector(DISK* disk) {
    return DISK_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

uint FAT_ReadFat(DISK* disk) {
    return DISK_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

uint FAT_ReadRootDirectory(DISK* disk) {
    uint32_t lba = g_Data->BS.BootSector.ReservedSectors + g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
    uint32_t size = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
    uint32_t sectors = (size + g_Data->BS.BootSector.BytesPerSector - 1) / g_Data->BS.BootSector.BytesPerSector;

    g_RootDirectoryEnd = lba + sectors;
    return DISK_ReadSectors(disk, lba, sectors, g_RootDirectory);
}

uint FAT_Initialize(DISK* disk) {
    g_Data = (FAT_Data far*)MEMORY_FAT_ADDR;

    uint err = FAT_ReadBootSector(disk);
    if (err) {
        printf("FAT: read boot sector failed\r\n");
        return err;
    }

    g_Fat = (uint8_t far*)g_Data + sizeof(FAT_Data);
    uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector * g_Data->BS.BootSector.SectorsPerFat;
    if (sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE) {
        printf("FAT: not enough memory to read FAT! Required %lu, only have %u\r\n", sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    err = FAT_ReadFat(disk);
    if (err) {
        printf("FAT: read FAT failed\r\n");
        return err;
    }

    g_RootDirectory = (FAT_DirectoryEntry far*)(g_Fat + fatSize);
    uint32_t rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
    rootDirSize = align(rootDirSize, g_Data->BS.BootSector.BytesPerSector);

    if (sizeof(FAT_Data) + fatSize + rootDirSize >= MEMORY_FAT_SIZE) {
        printf("FAT: not enough memory to read root directory! Required %lu, only have %u\r\n", sizeof(FAT_Data) + fatSize + rootDirSize, MEMORY_FAT_SIZE);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    err = FAT_ReadRootDirectory(disk);
    if (err) {
        printf("FAT: read root directory failed\r\n");
        return err;
    }
}

FAT_File* FAT_Open(DISK* disk, const char* path) {
    
}




// typedef struct {
//     uint8_t Name[11];
//     uint8_t Attributes;
//     uint8_t _Reserved;
//     uint8_t CreatedTimeTenths;
//     uint16_t CreatedTime;
//     uint16_t CreatedDate;
//     uint16_t AccessedDate;
//     uint16_t FirstClusterHigh;
//     uint16_t ModifiedTime;
//     uint16_t ModifiedDate;
//     uint16_t FirstClusterLow;
//     uint32_t Size;
// } __attribute__((packed)) DirectoryEntry;


// BootSector g_BootSector;
// uint8_t* g_Fat = NULL;
// DirectoryEntry* g_RootDirectory = NULL;
// uint32_t g_RootDirectoryEnd;

// uint8_t readBootSector(FILE* disk) {
//     return fread(&g_BootSector, sizeof(g_BootSector), 1, disk);
// }

// uint8_t readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut) {
//     uint8_t ok = 1;
//     ok = ok && (fseek(disk, lba * g_BootSector.BytesPerSector, SEEK_SET) == 0);
//     ok = ok && (fread(bufferOut, g_BootSector.BytesPerSector, count, disk));
//     return ok;
// }

// uint8_t readFat(FILE* disk) {
//     g_Fat = (uint8_t*)malloc(g_BootSector.SectorsPerFat * g_BootSector.BytesPerSector);
//     return readSectors(disk, g_BootSector.ReservedSectors, g_BootSector.SectorsPerFat, g_Fat);
// }

// uint8_t readRootDirectory(FILE* disk) {
//     uint32_t lba = g_BootSector.ReservedSectors + g_BootSector.SectorsPerFat * g_BootSector.FatCount;
//     uint32_t size = sizeof(DirectoryEntry) * g_BootSector.DirEntryCount;
//     uint32_t sectors = (size / g_BootSector.BytesPerSector);
//     if (size % g_BootSector.BytesPerSector) {
//         sectors++;
//     }

//     g_RootDirectoryEnd = lba + sectors;
//     g_RootDirectory = (DirectoryEntry*)malloc(sectors * g_BootSector.BytesPerSector);
//     return readSectors(disk, lba, sectors, g_RootDirectory);
// }

// DirectoryEntry* findFile(const char* name) {
//     for (uint32_t i = 0; i < g_BootSector.DirEntryCount; i++) {
//         if (memcmp(name, g_RootDirectory[i].Name, 11) == 0) {
//             return &g_RootDirectory[i];
//         }
//     }

//     return NULL;
// }

// uint8_t readFile(DirectoryEntry* fileEntry, FILE* disk, uint8_t* outputBuffer) {
//     uint8_t ok = 1;
//     uint16_t currentCluster = fileEntry->FirstClusterLow;

//     do {
//         uint32_t lba = g_RootDirectoryEnd + (currentCluster - 2) * g_BootSector.SectorsPerCluster;
//         ok = ok && readSectors(disk, lba, g_BootSector.SectorsPerCluster, outputBuffer);
//         outputBuffer += g_BootSector.SectorsPerCluster * g_BootSector.BytesPerSector;

//         uint32_t fatIndex = currentCluster * 3 / 2;
//         if (currentCluster % 2 == 0) {
//             currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) & 0x0FFF;
//         } else {
//             currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) >> 4;
//         }
//     } while (ok && currentCluster < 0x0FF8);

//     return ok;
// }

// int main(int argc, char** argv) {
//     if (argc < 3) {
//         printf("Syntax: %s <disk_image> <file_name>\n", argv[0]);
//     }

//     FILE* disk = fopen(argv[1], "rb");
//     if (!disk) {
//        fprintf(stderr, "Cannot open disk image %s!\n", argv[1]);
//        return -1;
//     }

//     if (!readBootSector(disk)) {
//         fprintf(stderr, "Could not read boot sector!\n");
//         return -2;
//     }

//     if (!readFat(disk)) {
//         fprintf(stderr, "Could not read FAT!\n");
//         free(g_Fat);
//         return -3;
//     }

//     if (!readRootDirectory(disk)) {
//         fprintf(stderr, "Could not read FAT!\n");
//         free(g_Fat);
//         free(g_RootDirectory);
//         return -4;
//     }

//     DirectoryEntry* fileEntry = findFile(argv[2]);
//     if (!fileEntry) {
//         fprintf(stderr, "Could not find file %s!\n", argv[2]);
//         free(g_Fat);
//         free(g_RootDirectory);
//         return -5;
//     }

//     uint8_t* buffer = (uint8_t*)malloc(fileEntry->Size + g_BootSector.BytesPerSector);
//     if (!readFile(fileEntry, disk, buffer)) {
//         fprintf(stderr, "Could not read file %s!\n", argv[2]);
//         free(g_Fat);
//         free(g_RootDirectory);
//         free(buffer);
//         return -6;
//     }

//     for (uint32_t i = 0; i < fileEntry->Size; i++) {
//         if (isprint(buffer[i])) {
//             fputc(buffer[i], stdout);
//         } else {
//             printf("<%02x>", buffer[i]);
//         }
//     }
//     printf("\n");

//     free(g_Fat);
//     free(g_RootDirectory);
//     free(buffer);
//     return 0;
// }