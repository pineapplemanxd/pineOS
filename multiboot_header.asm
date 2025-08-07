section .multiboot_header
align 4

; Multiboot header constants
MULTIBOOT_MAGIC    equ 0x1BADB002
MULTIBOOT_FLAGS    equ 0x00000003
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Multiboot header
multiboot_header:
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

global _start
extern kernel_main

section .text
align 4
_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Call kernel main
    call kernel_main
    
    ; Halt if kernel returns
    cli
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KB stack
stack_top: 