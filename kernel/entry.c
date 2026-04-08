#include <driver/uart/early_uart.h>
#include <driver/vga/early_vga.h>
#include <printk.h>
#include <arch/x86/io.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/irq.h>
#include <arch/x86/gdt.h>
#include <string.h>
#include <arch/x86/e820.h>
#include <arch/x86/bootmem.h>
#include <arch/x86/vmm.h>

extern uint8_t __bss_start;
extern uint8_t __bss_end;

#define VGA_FRAMEBUFFER ((unsigned char*)0xB8000)
#define VGA_FRAMEBUFFER_HHDM ((unsigned char*)0xFFFF8880000B8000)

#ifdef DEBUG
// Paints the stack to estimate usage and help detecting stack underflows
#define __MARK_STACK(s, o) memset((s), 0xDF, sizeof(s)-(o));
#define __STACK_USED(s) ({          \
    int used = 0;                   \
    while(used < sizeof(s)) {       \
        if (s[used] != 0xDF) break; \
        used++;                     \
    }                               \
    sizeof(s) - used;               \
})
#endif

static __attribute__((aligned(16))) unsigned char stack[4096];

void __attribute__((noreturn, section(".entry"))) entry(struct e820_table* e820_table) {
    cli();
    memset(&__bss_start, 0, (&__bss_end) - (&__bss_start));
    #ifdef DEBUG
    __MARK_STACK(stack, 0);
    #endif
    __asm__ volatile("mov %0, %%rsp" :: "r"(stack + sizeof(stack)) : "memory");

    early_uart_init();
    early_vga_clrscr();
    early_vga_init(VGA_FRAMEBUFFER);
    printk_sink_register((printk_sink_t){.name = "UART    ", .write = early_uart_log_write});
    printk_sink_register((printk_sink_t){.name = "VGA     ", .write = early_vga_log_write});
    printk("Kernel loaded at %#p\n", entry);
    printk("[OK] printk() active\n");

    gdt_init();
    idt_init();
    isr_init();
    irq_init();

    vmm_init();
    e820_init(e820_table);
    bootmem_init();
    vmm_map_hhdm();
    early_vga_init(VGA_FRAMEBUFFER_HHDM);
    vmm_unmap_low_identity();

    sti();

    #ifdef DEBUG
    printk(KERN_NOTICE "[NOTICE] Stack used %d B\n", __STACK_USED(stack));
    #endif
    while (1) halt();
}
