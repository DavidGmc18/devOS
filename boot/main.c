#include <stdint.h>
#include <memory.h>
#include <driver/vga/vga_text.h>
#include <arch/i686/printk.h>

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

    for (;;) ;
}