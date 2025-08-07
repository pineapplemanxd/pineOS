#include "io.h"
#include "kernel.h"

// VGA memory address
#define VGA_MEMORY 0xB8000

// VGA I/O ports
#define VGA_AC_INDEX 0x3C0
#define VGA_AC_WRITE 0x3C0
#define VGA_AC_READ  0x3C1
#define VGA_MISC_WRITE 0x3C2
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA  0x3C5
#define VGA_DAC_READ_INDEX 0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA 0x3C9
#define VGA_MISC_READ 0x3CC
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_DATA  0x3CF
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5
#define VGA_INSTAT_READ 0x3DA

// Keyboard I/O ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Serial I/O ports
#define SERIAL_COM1 0x3F8

// Global variables
static unsigned char* vga_buffer = (unsigned char*)VGA_MEMORY;
static int vga_x = 0;
static int vga_y = 0;
static unsigned char vga_color = VGA_LIGHT_GREY;

// Port I/O functions
void outb(unsigned short port, unsigned char value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outw(unsigned short port, unsigned short value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

unsigned short inw(unsigned short port) {
    unsigned short ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outl(unsigned short port, unsigned int value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

unsigned int inl(unsigned short port) {
    unsigned int ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// VGA functions
void vga_init(void) {
    vga_x = 0;
    vga_y = 0;
    vga_color = VGA_LIGHT_GREY;
    vga_clear();
}

void vga_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i * 2] = ' ';
        vga_buffer[i * 2 + 1] = vga_color;
    }
    vga_x = 0;
    vga_y = 0;
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_x = 0;
        vga_y++;
        if (vga_y >= VGA_HEIGHT) {
            vga_scroll();
        }
        return;
    }
    
    if (c == '\b') {
        if (vga_x > 0) {
            vga_x--;
        } else if (vga_y > 0) {
            vga_y--;
            vga_x = VGA_WIDTH - 1;
        }
        vga_buffer[(vga_y * VGA_WIDTH + vga_x) * 2] = ' ';
        return;
    }
    
    if (vga_x >= VGA_WIDTH) {
        vga_x = 0;
        vga_y++;
    }
    
    if (vga_y >= VGA_HEIGHT) {
        vga_scroll();
    }
    
    int index = (vga_y * VGA_WIDTH + vga_x) * 2;
    vga_buffer[index] = c;
    vga_buffer[index + 1] = vga_color;
    vga_x++;
}

void vga_puts(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

void vga_set_color(unsigned char color) {
    vga_color = color;
}

void vga_set_cursor(int x, int y) {
    vga_x = x;
    vga_y = y;
}

void vga_scroll(void) {
    // Move all lines up by one
    for (int i = 0; i < VGA_HEIGHT - 1; i++) {
        for (int j = 0; j < VGA_WIDTH; j++) {
            int src_index = ((i + 1) * VGA_WIDTH + j) * 2;
            int dst_index = (i * VGA_WIDTH + j) * 2;
            vga_buffer[dst_index] = vga_buffer[src_index];
            vga_buffer[dst_index + 1] = vga_buffer[src_index + 1];
        }
    }
    
    // Clear the last line
    for (int j = 0; j < VGA_WIDTH; j++) {
        int index = ((VGA_HEIGHT - 1) * VGA_WIDTH + j) * 2;
        vga_buffer[index] = ' ';
        vga_buffer[index + 1] = vga_color;
    }
    
    vga_y = VGA_HEIGHT - 1;
}

// Keyboard functions
void keyboard_init(void) {
    // Wait for keyboard to be ready
    while (inb(KEYBOARD_STATUS_PORT) & 0x02);
    
    // Send command to enable keyboard
    outb(KEYBOARD_STATUS_PORT, 0xAE);
}

char keyboard_read(void) {
    // Wait for data to be available
    while (!(inb(KEYBOARD_STATUS_PORT) & 0x01));
    
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);
    
    // Convert scancode to ASCII (simplified)
    switch (scancode) {
        case 0x1C: return '\n';  // Enter
        case 0x0E: return '\b';  // Backspace
        case 0x02: return '1';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';
        case 0x10: return 'q';
        case 0x11: return 'w';
        case 0x12: return 'e';
        case 0x13: return 'r';
        case 0x14: return 't';
        case 0x15: return 'y';
        case 0x16: return 'u';
        case 0x17: return 'i';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x1E: return 'a';
        case 0x1F: return 's';
        case 0x20: return 'd';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';
        case 0x2C: return 'z';
        case 0x2D: return 'x';
        case 0x2E: return 'c';
        case 0x2F: return 'v';
        case 0x30: return 'b';
        case 0x31: return 'n';
        case 0x32: return 'm';
        case 0x33: return ',';  // Comma
        case 0x34: return '.';  // Period
        case 0x35: return '/';  // Slash
        case 0x27: return ';';  // Semicolon
        case 0x28: return '\''; // Apostrophe
        case 0x1A: return '[';  // Left bracket
        case 0x1B: return ']';  // Right bracket
        case 0x0C: return '-';  // Minus
        case 0x0D: return '=';  // Equals
        case 0x29: return '`';  // Backtick
        case 0x2B: return '\\'; // Backslash
        case 0x39: return ' ';
        default: return 0;
    }
}

int keyboard_available(void) {
    return (inb(KEYBOARD_STATUS_PORT) & 0x01) != 0;
}

// Serial functions
void serial_init(void) {
    outb(SERIAL_COM1 + 1, 0x00);    // Disable all interrupts
    outb(SERIAL_COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(SERIAL_COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(SERIAL_COM1 + 1, 0x00);    //                  (hi byte)
    outb(SERIAL_COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(SERIAL_COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void serial_putchar(char c) {
    while ((inb(SERIAL_COM1 + 5) & 0x20) == 0);
    outb(SERIAL_COM1, c);
}

void serial_puts(const char* str) {
    while (*str) {
        serial_putchar(*str++);
    }
}

char serial_read(void) {
    while ((inb(SERIAL_COM1 + 5) & 0x01) == 0);
    return inb(SERIAL_COM1);
} 