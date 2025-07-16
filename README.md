# Simple Operating System

A basic operating system written in C and assembly for x86 architecture.

## Features

- **Bootloader**: Simple bootloader that loads the kernel from disk
- **Kernel**: Basic kernel with command-line interface
- **Memory Management**: Simple memory allocator with best-fit algorithm
- **Process Management**: Basic process creation and scheduling
- **I/O System**: VGA display and keyboard input support
- **Command Shell**: Interactive command-line interface

## Commands

The OS provides a simple command shell with the following commands:

- `help` - Show available commands
- `clear` - Clear the screen
- `memory` - Show memory status
- `process` - Show process status
- `test` - Run memory tests
- `reboot` - Reboot the system

## Building

### Prerequisites

You need the following tools installed:

- **GCC** (GNU Compiler Collection) with 32-bit support
- **NASM** (Netwide Assembler)
- **QEMU** (for emulation)
- **Make** (for building)

### On Windows

1. Install MinGW-w64 with 32-bit support
2. Install NASM
3. Install QEMU
4. Add all tools to your PATH

### On Linux

```bash
# Ubuntu/Debian
sudo apt-get install gcc-multilib nasm qemu-system-x86 make

# Fedora/RHEL
sudo dnf install gcc-multilib nasm qemu-system-x86 make
```

### On macOS

```bash
# Using Homebrew
brew install gcc nasm qemu make
```

## Compilation

To build the OS:

```bash
make
```

This will create:
- `bootloader.bin` - The bootloader binary
- `kernel.bin` - The kernel binary
- `os.img` - The complete OS disk image

## Running

To run the OS in QEMU:

```bash
make run
```

Or manually:

```bash
qemu-system-i386 -fda os.img -m 16
```

## Project Structure

```
os/
├── Makefile          # Build configuration
├── linker.ld         # Linker script
├── boot/
│   └── boot.asm      # Bootloader (assembly)
├── kernel/
│   ├── kernel.h      # Main kernel header
│   ├── kernel.c      # Main kernel implementation
│   ├── io.h          # I/O system header
│   ├── io.c          # I/O system implementation
│   ├── memory.h      # Memory management header
│   ├── memory.c      # Memory management implementation
│   ├── process.h     # Process management header
│   └── process.c     # Process management implementation
└── README.md         # This file
```

## Architecture

### Memory Layout

- `0x0000-0x07FF`: Bootloader (512 bytes)
- `0x1000-0x7FFF`: Kernel
- `0x8000-0x8FFF`: Stack
- `0x9000-0xFFFF`: Kernel data
- `0x10000-0xFFFFF`: User memory (1MB total)

### Components

1. **Bootloader**: Loads the kernel from disk and jumps to it
2. **Kernel**: Main OS code with initialization and main loop
3. **I/O System**: Handles VGA display and keyboard input
4. **Memory Manager**: Simple memory allocation and deallocation
5. **Process Manager**: Basic process creation and scheduling

## Limitations

This is a simplified OS for educational purposes:

- No file system
- No networking
- No graphics beyond text mode
- No interrupts (except for I/O)
- No virtual memory
- No user space protection
- Limited to 16MB of RAM

## Development

To modify the OS:

1. Edit the source files in the `kernel/` directory
2. Run `make clean` to clean previous builds
3. Run `make` to rebuild
4. Run `make run` to test

## Troubleshooting

### Build Errors

- **"gcc: command not found"**: Install GCC with 32-bit support
- **"nasm: command not found"**: Install NASM
- **"qemu-system-i386: command not found"**: Install QEMU

### Runtime Issues

- **OS doesn't boot**: Check that all files are compiled correctly
- **No keyboard input**: QEMU keyboard focus issues - click in the window
- **Screen corruption**: May indicate memory issues in the kernel

## Learning Resources

This OS demonstrates basic OS concepts:

- Boot process and BIOS interaction
- Memory management
- Process scheduling
- I/O programming
- System programming in C
- Assembly programming for low-level operations

## License

This project is for educational purposes. Feel free to use and modify as needed. 