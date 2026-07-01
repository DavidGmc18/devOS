#include <printk.h>
#include <string.h>

#include <driver/uart/early_uart.h>
#include <driver/vga/early_vga.h>

#include <arch/x86/io.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/irq.h>
#include <arch/x86/gdt.h>
#include <arch/x86/e820.h>
#include <arch/x86/bootmem.h>
#include <arch/x86/vmm.h>
#include <arch/x86/pit.h>

#include <addr.h>
#include <mm.h>

#include <mm/stack.h>
#include <mm/mem.h>

extern uint8_t __bss_start;
extern uint8_t __bss_end;

#define VGA_FRAMEBUFFER ((unsigned char*)0xB8000)
#define VGA_FRAMEBUFFER_HHDM ((unsigned char*)0xFFFF8880000B8000)

void user_test() {
    __asm__ volatile ("mov $0, %%rax" ::: "rax");
    while (1) {
        __asm__ volatile ("inc %%rax" ::: "rax");
    }
}

void __attribute__((noreturn, section(".entry"))) entry(struct e820_table* e820_table) {
    cli();
    memset(&__bss_start, 0, (&__bss_end) - (&__bss_start));
    #ifdef DEBUG
    __MARK_STACK(kern_stack, 0);
    #endif
    __SET_STACK(kern_stack);

    early_uart_init();
    early_vga_init(VGA_FRAMEBUFFER);
    early_vga_clrscr();
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

    mem_init();
    buddy_init();
    vmm_use_alloc_pages();

    pit_set_freq(1024);

    #ifdef DEBUG
    printk(KERN_NOTICE "[NOTICE] Used %d B of the stack\n", __STACK_USED(kern_stack));
    #endif

    sti();

    // User space
    uintptr_t user_virt = 0x100000;

    struct page* user_pages = alloc_pages(1);
    
    // TODO text should not be writable
    uint64_t* user_pml4 = vmm_create_user_pml4();
    vmm_map(user_pml4, user_virt, (uintptr_t)hhdm_to_phys(page_to_hhdm(user_pages)), 2*PAGE_SIZE, PT_PRESENT | PT_WRITABLE | PT_USER);
    vmm_set_table(user_pml4);

    memcpy((void*)(user_virt + 0x1000), user_test, 0x1000);

    __asm__ volatile (
        "mov %0, %%ds \n"
        "mov %0, %%es \n"
        
        "pushq %q0 \n"
        "pushq %q3 \n" // RSP
        "pushfq \n" // RFLAGS
        "pushq %q1 \n" // CS
        "pushq %q2 \n" // RIP (target)

        "iretq \n"
        ::
        "r" ((uint64_t)(GDT_USER_DATA_SEGMENT | 3)),
        "r" ((uint64_t)(GDT_USER_CODE_SEGMENT | 3)),
        "r" ((uint64_t)(user_virt + 0x1000)), // Text base
        "r" ((uint64_t)(user_virt + 0x1000)) // Stack top
        : "rax", "memory"
    );

    while (1) halt();
}
