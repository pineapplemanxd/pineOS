[org 0x7c00]
[bits 16]

; Initialize segments
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00

; Save boot drive number
mov [boot_drive], dl

; Load kernel from disk
mov ah, 0x02    ; BIOS read sector function
mov al, 30      ; Number of sectors to read
mov ch, 0       ; Cylinder 0
mov cl, 2       ; Sector 2 (sector 1 is bootloader)
mov dh, 0       ; Head 0
mov dl, [boot_drive]  ; Use saved drive number
mov bx, 0x1000  ; Load to address 0x1000
int 0x13        ; BIOS interrupt

; Check if read was successful
jc disk_error

; Print success message
mov si, success_msg
call print_string

; Switch to 32-bit mode
cli             ; Disable interrupts
lgdt [gdt_descriptor] ; Load GDT

; Enable A20 line
in al, 0x92
or al, 2
out 0x92, al

; Set PE bit in CR0
mov eax, cr0
or eax, 1
mov cr0, eax

; Jump to 32-bit code
jmp 0x08:protected_mode

disk_error:
    mov si, disk_error_msg
    call print_string
    jmp $

print_string:
    mov ah, 0x0e    ; BIOS teletype function
.loop:
    lodsb           ; Load next character
    test al, al     ; Check if character is null
    jz .done        ; If null, we're done
    int 0x10        ; Print character
    jmp .loop       ; Repeat
.done:
    ret

[bits 32]
protected_mode:
    ; Set up segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up stack
    mov esp, 0x9000
    
    ; Jump to kernel
    jmp 0x1000

disk_error_msg db 'Disk read error!', 0
success_msg db 'Kernel loaded successfully!', 0
boot_drive db 0

; GDT
gdt_start:
    dq 0x0000000000000000  ; Null descriptor
    dq 0x00CF9A000000FFFF  ; Code descriptor
    dq 0x00CF92000000FFFF  ; Data descriptor
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Boot signature
times 510-($-$$) db 0
dw 0xaa55 