#pragma once

#include <stdint.h>

typedef struct {
    uint16_t bytes_per_sector;    
    uint8_t sectors_per_cluster;  
    uint16_t reserved_sectors;    
    uint8_t fat_count;            
    uint16_t root_dir_entries;    
    uint16_t total_sectors;       
    uint8_t media_descriptor;     
    uint16_t sectors_per_fat;     
    uint16_t sectors_per_track;   
    uint16_t head_count;          
    uint32_t hidden_sectors;      
    uint32_t large_sector_count;
} __attribute__((packed)) _BPB;

typedef struct {
    uint8_t drive_number;
    uint8_t flags;
    uint8_t boot_signature;
    uint32_t volume_id;
} __attribute__((packed)) _EBPB;

const extern char OEM_ID[8];
const extern _BPB BPB;
const extern _EBPB EBPB;