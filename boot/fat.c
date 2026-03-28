#include "fat.h"
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

typedef struct {
    // 0x0B
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_dir_entries_count;
    uint16_t total_sectors;
    uint8_t media_descriptor;
    uint16_t __sectors_per_fat;
    uint16_t sector_per_track;
    uint16_t heads_count;
    uint32_t hidden_sector_count;
    uint32_t large_sector_count;
    
    // 0x24
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fat_version_num;
    uint32_t root_dir_cluster;
    uint16_t fsi_sector;
    uint16_t backup_boot_sector_sector;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved_flags;
    uint8_t signature;
    uint32_t volume_id;
    char volume_label[11];
    char sys_id[8];
} __attribute__((packed)) fat_bpb_t;

typedef struct {
    char oem_id[8];
    fat_bpb_t bpb;
} __attribute__((packed)) fat_header_t;

__attribute__((section(".fat_header"), used, aligned(1))) fat_header_t fat_header;
_Static_assert(sizeof(fat_header_t) == 87, "wrong size");

static BL_DiskRead disk_read;

void fat_init(BL_DiskRead disk_read_fn) {
    disk_read = disk_read_fn;
}

static int fat_read_sectors(fat_dev_t* dev, uint32_t lba, uint32_t count, uint16_t* buffer) {
    if (lba + count > dev->sectors) return -1;
    return disk_read(dev->abar, dev->port, (BL_LBA48){dev->lba + lba}, count, buffer);
}

int fat_dev_init(fat_dev_t* dev, BL_Disk* disk) {
    dev->abar = disk->abar;
    dev->port = disk->port;
    dev->lba = disk->partition.lba;
    dev->sectors = disk->partition.sectors;

    uint16_t boot_sector[256];
    if (fat_read_sectors(dev, 0, 1, boot_sector)) return -1;

    fat_header_t* header = (fat_header_t*)((uint8_t*)boot_sector + 3);

    dev->fat = header->bpb.reserved_sectors;
    dev->fat_sectors = header->bpb.fat_count * header->bpb.sectors_per_fat;

    dev->root_dir_cluster = header->bpb.root_dir_cluster;
    dev->data = dev->fat + dev->fat_sectors;
    dev->sectors_per_cluster = header->bpb.sectors_per_cluster;

    return 0;
}

// FAT cache does not update if you performed writes to FAT on disk
static struct {
    bool valid;
    void* abar;
    uint8_t port;
    uint32_t lba;
    uint32_t fat;
    uint32_t fat_sectors;
    uint32_t sector_no;
    uint32_t entry[128];
} fat_cache = {0};

static bool fat_cache_is_stale(fat_dev_t* dev, uint32_t sector_no) {
    if (!fat_cache.valid)
        return true;

    if (
        fat_cache.abar != dev->abar ||
        fat_cache.port != dev->port ||
        fat_cache.lba != dev->lba ||
        fat_cache.fat != dev->fat ||
        fat_cache.fat_sectors != dev->fat_sectors
    ) return true;

    if (fat_cache.sector_no != sector_no)
        return true;

    return false;
}

#define RET_ERR(func) do { int _err = (func); if (_err) return _err; } while(0)

static int fat_cache_fetch(fat_dev_t* dev, uint32_t sector_no) {
    RET_ERR(fat_read_sectors(dev, dev->fat+sector_no, 1, (uint16_t*)fat_cache.entry));
    fat_cache.valid = true;
    fat_cache.abar = dev->abar;
    fat_cache.port = dev->port;
    fat_cache.lba = dev->lba;
    fat_cache.fat = dev->fat;
    fat_cache.fat_sectors = dev->fat_sectors;
    fat_cache.sector_no = sector_no;
    return 0;
}

static int fat_get_next_cluster(fat_dev_t* dev, uint32_t current_cluster_no, uint32_t* next_cluster) {
    uint32_t sector_no = current_cluster_no / 128;
    uint32_t local_offset = current_cluster_no % 128;

    if (fat_cache_is_stale(dev, sector_no))
        RET_ERR(fat_cache_fetch(dev, sector_no));

    *next_cluster =  fat_cache.entry[local_offset];
    return 0;
}

#define FAT_ATTR_DIRECTORY 0x10

typedef struct {
    char     name[8];
    char     ext[3];
    uint8_t  attributes;
    uint8_t  reserved;
    uint8_t  creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t cluster_low;
    uint32_t size;
} __attribute__((packed)) fat_dir_entry_t;

#define FAT_DIR_ENTRIES_PER_SECTOR (512 / sizeof(fat_dir_entry_t))

static uint16_t buffer[256];

static int fat_file_lookup(fat_dev_t* dev, fat_dir_entry_t* directory, const char* target, fat_dir_entry_t* file) {
    if (!(directory->attributes & FAT_ATTR_DIRECTORY)) return -1;

    uint32_t dir_cluster = directory->cluster_low | ((uint32_t)directory->cluster_high << 16);
    
    while (dir_cluster >= 2 && dir_cluster < 0x0FFFFFF8) {
        uint32_t lba = dev->data + (dir_cluster - 2) * dev->sectors_per_cluster;

        for (int s = 0; s < dev->sectors_per_cluster; s++) {
            RET_ERR(fat_read_sectors(dev, lba+s, 1, buffer));
            fat_dir_entry_t* dir_entries = (fat_dir_entry_t*)buffer;

            for (int i = 0; i < FAT_DIR_ENTRIES_PER_SECTOR; i++) {
                if (dir_entries[i].name[0] == 0x00) return -1; // last file
                if (dir_entries[i].name[0] == 0xE5) continue; // deleted
                if (dir_entries[i].attributes == 0x0F) continue; // LFN

                if (!memcmp(dir_entries[i].name, target, 11)) {
                    *file = dir_entries[i]; 
                    return 0;
                }
            }
        }

        RET_ERR(fat_get_next_cluster(dev, dir_cluster, &dir_cluster));
    }

    return -1;
}

static void fat_short_name_to_name_ext(const char* short_name, char name_ext[11]) {
    if (*short_name == '/') short_name++;

    int i = 0;
    while (i < 11) {
        // End of short name
        if (*short_name == '/' || *short_name == 0x0)
            break;

        // name - ext separator
        if (*short_name == '.') {
            if (i < 8) memset(name_ext+i, ' ', 8-i);
            i = 8;
            short_name++;
            continue;
        }

        name_ext[i++] = toupper(*short_name++);
    }
    
    memset(name_ext+i, ' ', 11-i);
}

static int fat_path_lookup(fat_dev_t* dev, fat_dir_entry_t* directory, const char* path, fat_dir_entry_t* file) {
    fat_dir_entry_t current_entry = *directory;
    while (path && *path) {
        char target[11];
        fat_short_name_to_name_ext(path, target);
        path = strchr(path, '/');
        if (path) path++;
        RET_ERR(fat_file_lookup(dev, &current_entry, target, &current_entry));
    }
    *file = current_entry;
    return 0;
}

int fat_read(fat_dev_t* dev, const char* path, void* buffer) {
    fat_dir_entry_t root = {
        .name = "ROOT    ",
        .ext = "   ",
        .attributes = FAT_ATTR_DIRECTORY,
        .cluster_high = (uint16_t)(dev->root_dir_cluster >> 16),
        .cluster_low = (uint16_t)(dev->root_dir_cluster & 0xFFFF)
    };
    fat_dir_entry_t file;
    RET_ERR(fat_path_lookup(dev, &root, path, &file));

    uint32_t current_cluster = file.cluster_low | ((uint32_t)file.cluster_high << 16);
    while (current_cluster >= 2 && current_cluster < 0x0FFFFFF8) {
        uint32_t lba = dev->data + (current_cluster - 2) * dev->sectors_per_cluster;
        RET_ERR(fat_read_sectors(dev, lba, dev->sectors_per_cluster, buffer));
        buffer = (uint8_t*)buffer + dev->sectors_per_cluster * 512;
        RET_ERR(fat_get_next_cluster(dev, current_cluster, &current_cluster));
    }
    return 0;
}