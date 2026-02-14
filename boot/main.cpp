#include <stdint.h>
#include <memory.h>
#include <driver/vga/vga_text.h>
#include <arch/i686/printk.h>
#include <system/storage/block/MBR.h>
#include <system/memory/page.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void __attribute__((cdecl)) main(uint16_t drive, uint8_t partition) {
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    VGA_Initialize(80, 25, (uint8_t*)0xB8000);
    VGA_clrscr();

    VGA_set_color(0xD0);
    printk("Davidak OS!!!\n");

    VGA_set_color(0x07);
    printk("Boot params -> drive=%d partition=%d\n", drive, partition);

    MBR_Table mbr_table;
    MBR_get_table(drive, &mbr_table);

    printk("LBA=%d sectors=%d\n", mbr_table.partitions[partition].lba, mbr_table.partitions[partition].sectors);

    PageBlock block1 = alloc_page(2);

    PageBlock block2 = alloc_page(10);

    free_page(&block1);

    PageBlock block3 = alloc_page(1);

    printk("PB1: ptr=0x%x size=%d\n", block1.ptr, block1.count);
    printk("PB2: ptr=0x%x size=%d\n", block2.ptr, block2.count);
    printk("PB3: ptr=0x%x size=%d\n", block3.ptr, block3.count);

    for (;;) ;
}