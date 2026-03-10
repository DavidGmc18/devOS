ENTRY(start)
OUTPUT_FORMAT("binary")
phys = 0x00100000;

MEMORY {
    BOOTLOADER (!rwx) : ORIGIN = 0x00000500, LENGTH = 0x00019B00
    RAM (rwx) :         ORIGIN = 0x00100000, LENGTH = 0x00100000
}

SECTIONS
{
    . = phys;

    .entry : {
        __entry_start = .;
        *(.entry)
    } > RAM

    .oem_id : {
        __oem_id_start = .;
        *(.oem_id)
    } > RAM

    .bpb : {
        __bpb_start = .;
        *(.bpb)
    } > RAM

    .ebpb : {
        __ebpb_start = .;
        *(.ebpb)
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