#include <bl/boot.h>
#include <string.h>

extern uint8_t __bss_start;
extern uint8_t __end;

extern uint8_t __entry_start;
extern uint8_t __oem_id_start;
extern uint8_t __bpb_start;
extern uint8_t __ebpb_start;
extern uint8_t __text_start;

void start(BL_BootInfo* boot_info, BL_BootServices* boot_services) {
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    boot_services->printk("TEST!!!\n");

    boot_services->printk("boot_info = {\n");
    boot_services->printk("  disk = {\n");
    boot_services->printk("    abar = 0x%x\n", boot_info->disk.abar);
    boot_services->printk("    port = %d\n", boot_info->disk.port);
    boot_services->printk("    partition = {\n");
    boot_services->printk("      id = %d\n", boot_info->disk.partition.id);
    boot_services->printk("      lba = %d\n", boot_info->disk.partition.lba);
    boot_services->printk("      sectors = %d\n", boot_info->disk.partition.sectors);
    boot_services->printk("    }\n");
    boot_services->printk("  drive_name = '%s'\n", boot_info->disk.drive_name);
    boot_services->printk("  memory_info = {\n");
    boot_services->printk("    block_count = %d\n", boot_info->memory_info.block_count);
    boot_services->printk("    blocks[256]\n");
    boot_services->printk("  }\n");
    boot_services->printk("}\n");

    uint16_t buffer[256];
    boot_services->disk_read(boot_info->disk.abar, boot_info->disk.port, (BL_LBA48){0}, 1, buffer);
    boot_services->printk("Test disk read => 0x%x\n", buffer[255]);

    boot_services->printk("0x%x 0x%x 0x%x 0x%x 0x%x\n", &__entry_start, &__oem_id_start, &__bpb_start, &__ebpb_start,  &__text_start);

    
    
    while (1) __asm__ volatile ("hlt" ::: "memory");
}