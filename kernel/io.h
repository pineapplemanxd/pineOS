#ifndef IO_H
#define IO_H

// VGA I/O functions
void vga_init(void);
void vga_clear(void);
void vga_putchar(char c);
void vga_puts(const char* str);
void vga_set_color(unsigned char color);
void vga_set_cursor(int x, int y);
void vga_scroll(void);

// Keyboard I/O functions
void keyboard_init(void);
char keyboard_read(void);
int keyboard_available(void);

// Port I/O functions
void outb(unsigned short port, unsigned char value);
unsigned char inb(unsigned short port);
void outw(unsigned short port, unsigned short value);
unsigned short inw(unsigned short port);

// Serial I/O functions
void serial_init(void);
void serial_putchar(char c);
void serial_puts(const char* str);
char serial_read(void);

#endif 