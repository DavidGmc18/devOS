ENTRY(entry)
OUTPUT_FORMAT("binary")
phys = 0x00100000;

SECTIONS
{
    . = phys;

    .entry : {
        __entry_start = .;
        *(.entry)
    }

    .oem_id : {
        *(.oem_id)
    }

    .bpb : {
        *(.bpb)
    }

    .ebpb : {
        *(.ebpb)
    }

    .start : {
        *(.start)
    }

    .text : {
        __text_start = .;
        *(.text)
    }

    .data : {
        __data_start = .;
        *(.data)
    }

    .rodata : {
        __rodata_start = .;
        *(.rodata)
    }

    .bss : {
        __bss_start = .;
        *(.bss)
    }
    
    __end = .;
}