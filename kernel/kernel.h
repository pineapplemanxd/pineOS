#ifndef KERNEL_H
#define KERNEL_H

#include "io.h"
#include "memory.h"
#include "process.h"
#include "filesystem.h"

// Kernel entry point
void _start(void);

// Kernel initialization
void kernel_init(void);

// Main kernel loop
void kernel_main(void);

// System calls are now defined in user.h

// Colors for VGA
#define VGA_BLACK         0
#define VGA_BLUE          1
#define VGA_GREEN         2
#define VGA_CYAN          3
#define VGA_RED           4
#define VGA_MAGENTA       5
#define VGA_BROWN         6
#define VGA_LIGHT_GREY    7
#define VGA_DARK_GREY     8
#define VGA_LIGHT_BLUE    9
#define VGA_LIGHT_GREEN   10
#define VGA_LIGHT_CYAN    11
#define VGA_LIGHT_RED     12
#define VGA_LIGHT_MAGENTA 13
#define VGA_LIGHT_BROWN   14
#define VGA_WHITE         15

// VGA dimensions
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

// Memory layout
#define KERNEL_START 0x1000
#define KERNEL_END   0x8000
#define STACK_START  0x9000
#define STACK_END    0xA000

#endif 