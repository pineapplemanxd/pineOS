# pineOS Project

A simple educational operating system with a custom bootloader, kernel, and in-memory filesystem. Includes a shell with basic commands, written in C and x86 assembly, and runs in QEMU.

## Features
- Custom bootloader (x86 assembly)
- Flat binary kernel (C)
- In-memory filesystem with persistent storage support
- USB storage device detection and management
- Shell with commands: `ls`, `cd`, `mkdir`, `touch`, `cat`, `echo`, `rm`, `rmdir`, `tree`, `cp`
- Storage commands: `storage`, `save`, `load`, `format`
- Minimal process and memory management
- Bootable from USB drives
- Runs in QEMU (x86) and real hardware

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
   make os.iso
   ```
   This will:
   - Build the kernel (kernel/*.c)
   - Create a bootable ISO image (`os.iso`)

3. **Create USB-bootable image:**
   ```sh
   ./create_usb_simple.sh
   ```
   This creates `pineos_usb.img` that can be written to a USB drive.

4. **Run the OS in QEMU:**
   ```sh
   qemu-system-i386 -cdrom os.iso -m 32
   ```
   Or test the USB image:
   ```sh
   qemu-system-i386 -drive file=pineos_usb.img,format=raw -m 32
   ```
   You should see the shell prompt: `> `

## Usage

- Type `help` to see available commands.
- **File system commands:**
  - `ls` — List directory contents
  - `cd dir` — Change directory
  - `mkdir dir` — Create directory
  - `touch file` — Create empty file
  - `cat file` — Show file contents
  - `echo file content` — Write content to file
  - `rm file` — Remove file
  - `rmdir dir` — Remove directory (must be empty)
  - `tree` — Show directory tree
  - `cp src dest` — Copy file
- **Storage management commands:**
  - `storage` — List detected storage devices
  - `save` — Save current filesystem to USB storage
  - `load` — Load filesystem from USB storage
  - `format` — Format USB storage device

## USB Boot Instructions

1. **Create the USB image:**
   ```sh
   ./create_usb_simple.sh
   ```

2. **Write to USB drive:**
   - **On Linux:**
     ```sh
     sudo dd if=pineos_usb.img of=/dev/sdX bs=1M status=progress
     ```
     (Replace `/dev/sdX` with your USB device)
   
   - **On Windows:**
     1. Download [Rufus](https://rufus.ie/)
     2. Select your USB drive
     3. Choose "DD Image" mode
     4. Select the `pineos_usb.img` file
     5. Click START

3. **Boot from USB:**
   - Insert the USB drive into your laptop
   - Boot from USB (usually F12 or F2 during startup)
   - Select the USB drive from the boot menu

## Special Notes
- Only lowercase letters, numbers, and some special characters are supported in the shell (see `kernel/io.c` for details).
- The filesystem supports both in-memory operation and persistent storage to USB devices.
- Use `save` command to persist your filesystem to USB storage, and `load` to restore it.
- The OS is for educational/demo purposes but can run on real hardware via USB boot.

## Troubleshooting
- If you see only "Kernel loaded successfully!" and nothing else, check that your kernel is being built as a flat binary and the entry point is correct.
- If you cannot type certain characters, add their scancodes to `keyboard_read` in `kernel/io.c`.
- For boot or build issues, ensure all dependencies are installed and you are using the provided build scripts.

## License
This project is for educational use. No warranty is provided.
