ENTRY(entry)
OUTPUT_FORMAT("binary")

MEMORY {
    RAM (rwx) :         ORIGIN = 0x00100000, LENGTH = 0x00100000
}

SECTIONS {
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
}