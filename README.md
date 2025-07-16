# Simple OS Project

A simple educational operating system with a custom bootloader, kernel, and in-memory filesystem. Includes a shell with basic commands, written in C and x86 assembly, and runs in QEMU.

## Features
- Custom bootloader (x86 assembly)
- Flat binary kernel (C)
- In-memory filesystem (files and directories)
- Shell with commands: `ls`, `cd`, `mkdir`, `touch`, `cat`, `echo`, `rm`, `rmdir`, `tree`, `mv`
- Minimal process and memory management
- Runs in QEMU (x86)

## Requirements
- GCC (with 32-bit support)
- NASM
- QEMU
- Bash (for build scripts)

## Build Instructions

1. **Install dependencies:**
   - On Ubuntu/Debian:
     ```sh
     sudo apt update
     sudo apt install build-essential nasm qemu-system-x86
     ```
   - On Windows: Use WSL or install MinGW, NASM, and QEMU.

2. **Build the OS:**
   ```sh
   ./build_simple.sh
   ```
   This will:
   - Build the bootloader (boot/boot.asm)
   - Build the kernel (kernel/*.c, kernel/kernel_entry.asm)
   - Create a floppy disk image (`os.img`)

3. **Run the OS in QEMU:**
   ```sh
   qemu-system-i386 -fda os.img -m 16
   ```
   You should see the shell prompt: `> `

## Usage

- Type `help` to see available commands.
- Example commands:
  - `ls` — List directory contents
  - `cd dir` — Change directory
  - `mkdir dir` — Create directory
  - `touch file` — Create empty file
  - `cat file` — Show file contents
  - `echo file content` — Write content to file
  - `rm file` — Remove file
  - `rmdir dir` — Remove directory (must be empty)
  - `tree` — Show directory tree
  - `mv src dest` — Move or rename file/directory

## Special Notes
- Only lowercase letters, numbers, and some special characters are supported in the shell (see `kernel/io.c` for details).
- The filesystem is in-memory only (no persistence).
- The OS is for educational/demo purposes and not intended for real hardware.

## Troubleshooting
- If you see only "Kernel loaded successfully!" and nothing else, check that your kernel is being built as a flat binary and the entry point is correct.
- If you cannot type certain characters, add their scancodes to `keyboard_read` in `kernel/io.c`.
- For boot or build issues, ensure all dependencies are installed and you are using the provided build scripts.

## License
This project is for educational use. No warranty is provided.
