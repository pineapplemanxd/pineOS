ENTRY(_start)
SECTIONS
{
    . = 0x100000;
    
    .multiboot_header : ALIGN(4) {
        *(.multiboot_header)
    }
    
    .text : ALIGN(4) {
        *(.text)
    }
    
    .rodata : ALIGN(4) {
        *(.rodata)
    }
    
    .data : ALIGN(4) {
        *(.data)
    }
    
    .bss : ALIGN(4) {
        *(COMMON)
        *(.bss)
    }
} 