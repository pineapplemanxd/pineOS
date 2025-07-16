ENTRY(_start)
SECTIONS
{
    . = 0x1000;
    .multiboot_header : {
        *(.multiboot_header)
    }
    .text : {
        *(.text)
    }
    .rodata : {
        *(.rodata)
    }
    .data : {
        *(.data)
    }
    .bss : {
        *(.bss)
    }
} 