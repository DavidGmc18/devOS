ENTRY(start)
OUTPUT_FORMAT("binary")
phys = 0x110000;

MEMORY {
    RAM (rwx) :     ORIGIN = 0x110000,  LENGTH = 0x0E0000
    STACK (rw) :    ORIGIN = 0x1F0000,  LENGTH = 0x010000
}

SECTIONS {
    . = phys;

    .entry : {
        __entry_start = .;
        *(.entry)
    } > RAM

    .text : {
        __text_start = .;
        *(.text)
    } > RAM

    .data : {
        __data_start = .;
        *(.data)
    } > RAM

    .rodata : {
        __rodata_start = .;
        *(.rodata)
    } > RAM

    .bss : {
        __bss_start = .;
        *(.bss)
    } > RAM
    
    __end = .;
}