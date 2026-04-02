ENTRY(entry)
OUTPUT_FORMAT("binary")

KERNEL_PHYS = 0x200000;

MEMORY {
    RAM (rwx) : ORIGIN = 0xFFFFFFFF80000000, LENGTH = 0x200000
}

SECTIONS {
    . = ORIGIN(RAM);
    __kernel_vma_start = .;

    .text ORIGIN(RAM) : {
        *(.entry)
        *(.text)
    } > RAM

    .data : {
        *(.data)
    } > RAM

    .rodata : {
        *(.rodata)
    } > RAM

    .bss : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    } > RAM

    __kernel_vma_end = .;
}