#include "ata.h"
#include "io.h"
#include "stdio.h"

// ATA I/O port offsets
#define ATA_REG_DATA        0x0   // Data Register (16-bit)
#define ATA_REG_ERROR       0x1   // Error Register (read)
#define ATA_REG_FEATURES    0x1   // Features Register (write)
#define ATA_REG_SECCOUNT    0x2   // Sector count
#define ATA_REG_LBA_LOW     0x3   // LBA low byte
#define ATA_REG_LBA_MID     0x4   // LBA mid byte
#define ATA_REG_LBA_HIGH    0x5   // LBA high byte
#define ATA_REG_DRIVE       0x6   // Drive/Head register
#define ATA_REG_STATUS      0x7   // Status (read)
#define ATA_REG_COMMAND     0x7   // Command (write)

// Drive selection Primary/Secondary
// #define ATA_PRIMARY_BASE 0x1F0
// #define ATA_SECONDARY_BASE 0x170
#define ATA_BUSES_LEN 4
uint16_t ATA_BUSES[ATA_BUSES_LEN] = {
    0x1F0,
    0x170,
    0x1E8,
    0x168
};

// Control ports
#define ATA_PRIMARY_CTRL    0x3F6
#define ATA_SECONDARY_CTRL  0x376
#define ATA_CTRL_OFFSET     0x206

// Drive selection Master/Slave
#define ATA_MASTER 0x00
#define ATA_SLAVE 0x10

// Status bits
#define ATA_SR_BSY  0x80
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

// Control
#define ATA_CMD_CLEAR 0x00
#define ATA_CMD_SRST 0x04 // Software Reset
#define ATA_CMD_SELECT 0xA0
#define ATA_CMD_LBA 0xE0
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_SECTOR_WORDS 256

// TODO remake
// Send a 400 ns delay by reading alt status 4 times
static void ATA_io_delay(uint16_t ctrl_base) {
    i686_inb(ctrl_base); i686_inb(ctrl_base);
    i686_inb(ctrl_base); i686_inb(ctrl_base);
}

// Simple software reset
static void ATA_soft_reset(uint16_t bus) {
    uint16_t ctrl = bus + ATA_CTRL_OFFSET;

    i686_outb(ctrl, ATA_CMD_SRST);   // Set software reset
    ATA_io_delay(ctrl);           // ~400 ns delay
    i686_outb(ctrl, ATA_CMD_CLEAR);   // Clear reset

    // Wait until BSY clears
    uint8_t status;
    do {
        status = i686_inb(ctrl);  // read alternate status
    } while(status & ATA_SR_BSY);
}

enum ATA_DEV ATA_identify(uint16_t id, uint16_t* identify_buf) {
    uint16_t bus = id >> 1;
    // TODO check as well if given bus is supported by hardware
    if (bus >= ATA_BUSES_LEN) {
        printf("ATA: bus %d is out of range", bus);
        return ATA_DEV_UNKNOWN;
    }
    uint16_t base = ATA_BUSES[bus];
    uint8_t slave = (id & 1) << 4;

    ATA_soft_reset(base);

    i686_outb(base + ATA_REG_DRIVE, ATA_CMD_SELECT | slave);
    ATA_io_delay(base + ATA_CTRL_OFFSET);

    i686_outb(base + ATA_REG_SECCOUNT, 0);
    i686_outb(base + ATA_REG_LBA_LOW, 0);
    i686_outb(base + ATA_REG_LBA_MID, 0);
    i686_outb(base + ATA_REG_LBA_HIGH, 0);

    i686_outb(base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    uint8_t status = i686_inb(base + ATA_REG_STATUS);

    if (status == 0 || status == 0xFF)
        return ATA_DEV_NONE;

    while (status & ATA_SR_BSY) {
        status = i686_inb(base + ATA_REG_STATUS);
    }

    uint8_t mid  = i686_inb(base + ATA_REG_LBA_MID);
    uint8_t high = i686_inb(base + ATA_REG_LBA_HIGH);

    while (!(status & (ATA_SR_DRQ | ATA_SR_ERR))) {
        status = i686_inb(base + ATA_REG_STATUS);
    }

    if (status & ATA_SR_ERR)
        return ATA_DEV_UNKNOWN;

    for (int i = 0; i < 256; i++) {
        identify_buf[i] = i686_inw(base + ATA_REG_DATA);
    }

    if (mid != 0 || high != 0)
        return ATA_DEV_UNKNOWN;

    if (identify_buf[0] & 0xF000)
        return ATA_DEV_PATAPI;

    return ATA_DEV_PATA;
}

struct ATA_SLOT {
    uint16_t id;
    enum ATA_DEV type;
};

#define ATA_SLOTS 4
struct ATA_SLOT ATA_devices[ATA_SLOTS] = {
    {0, ATA_DEV_UNKNOWN},
    {1, ATA_DEV_UNKNOWN},
    {2, ATA_DEV_UNKNOWN},
    {3, ATA_DEV_UNKNOWN},
    // {4, ATA_DEV_UNKNOWN},
    // {5, ATA_DEV_UNKNOWN},
    // {6, ATA_DEV_UNKNOWN},
    // {7, ATA_DEV_UNKNOWN},
};

void ATA_scan_devices() {
    for (int i = 0; i < ATA_SLOTS; i++) {
        uint16_t identify_buffer[256];
        ATA_devices[i].type = ATA_identify(ATA_devices[i].id, identify_buffer);
    }
}

// no checks, lower-level
static void ATA_read28(struct ATA_SLOT slot, uint32_t LBA, uint8_t sectors, uint16_t* buffer) {
    uint16_t bus = slot.id >> 1;
    // TODO check as well if given bus is supported by hardware
    if (bus >= ATA_BUSES_LEN) {
        printf("ATA: bus %d is out of range", bus);
        return;
    }
    uint16_t base = ATA_BUSES[bus];
    uint8_t slave = (slot.id & 1) << 4;

    i686_outb(base + ATA_REG_DRIVE, ATA_CMD_LBA | slave | ((LBA >> 24) & 0x0F));
    i686_outb(base + ATA_REG_SECCOUNT, sectors);
    i686_outb(base + ATA_REG_LBA_LOW, LBA);
    i686_outb(base + ATA_REG_LBA_MID, LBA >> 8);
    i686_outb(base + ATA_REG_LBA_HIGH, LBA >> 16);

    i686_outb(base + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    for (int s = 0; s < sectors; s++) {
        // Wait for BSY=0 and DRQ=1
        uint8_t status;
        do {
            status = i686_inb(base + ATA_REG_STATUS);
        } while ((status & ATA_SR_BSY) || !(status & ATA_SR_DRQ));

        // Optional: check ERR
        if (status & ATA_SR_ERR) {
            // handle error, e.g., return or break
            return;
        }

        // 6. Read 256 words (512 bytes) from data port
        for (int w = 0; w < ATA_SECTOR_WORDS; w++) {
            buffer[s * ATA_SECTOR_WORDS + w] = i686_inw(base + ATA_REG_DATA);
        }
    }
};

void ATA_test() {
    ATA_scan_devices();

    const char* devnames[] = { "UNKNOWN", "NONE", "PATA", "PATAPI"};
    for (int i = 0; i < ATA_SLOTS; i++) {
        struct ATA_SLOT dev = ATA_devices[i];
        printf("ATA: DEV id=%d dev=%s\n", dev.id, devnames[dev.type]);
    }

    uint16_t buffer[256];

    ATA_read28(ATA_devices[0], 0, 1, buffer);

    print_buffer("", buffer, 512);
}