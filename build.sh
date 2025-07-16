#!/bin/bash

echo "Building Simple OS..."

# Check if required tools are available
if ! command -v gcc &> /dev/null; then
    echo "Error: GCC not found. Please install GCC with 32-bit support."
    exit 1
fi

if ! command -v nasm &> /dev/null; then
    echo "Error: NASM not found. Please install NASM."
    exit 1
fi

if ! command -v qemu-system-i386 &> /dev/null; then
    echo "Warning: QEMU not found. You can still build the OS but cannot run it."
    echo "Please install QEMU to test the OS."
fi

# Clean previous builds
rm -f *.bin *.img kernel/*.o

# Build bootloader
echo "Building bootloader..."
nasm -f bin -o bootloader.bin boot/boot.asm
if [ $? -ne 0 ]; then
    echo "Error: Failed to build bootloader"
    exit 1
fi

# Build kernel
echo "Building kernel..."
gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/kernel.o kernel/kernel.c
if [ $? -ne 0 ]; then
    echo "Error: Failed to build kernel.c"
    exit 1
fi

gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/io.o kernel/io.c
if [ $? -ne 0 ]; then
    echo "Error: Failed to build io.c"
    exit 1
fi

gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/memory.o kernel/memory.c
if [ $? -ne 0 ]; then
    echo "Error: Failed to build memory.c"
    exit 1
fi

gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/process.o kernel/process.c
if [ $? -ne 0 ]; then
    echo "Error: Failed to build process.c"
    exit 1
fi

# Link kernel
echo "Linking kernel..."
ld -m elf_i386 -T linker.ld -o kernel.elf kernel/kernel.o kernel/io.o kernel/memory.o kernel/process.o
if [ $? -ne 0 ]; then
    echo "Error: Failed to link kernel"
    exit 1
fi

# Convert ELF to raw binary
echo "Converting kernel to raw binary..."
objcopy -O binary kernel.elf kernel.bin
if [ $? -ne 0 ]; then
    echo "Error: Failed to convert kernel to binary"
    exit 1
fi

# Create disk image
echo "Creating disk image..."
dd if=/dev/zero of=os.img bs=512 count=2880 2>/dev/null
if [ $? -ne 0 ]; then
    echo "Error: Failed to create disk image"
    exit 1
fi

# Copy bootloader to disk image
dd if=bootloader.bin of=os.img conv=notrunc 2>/dev/null
if [ $? -ne 0 ]; then
    echo "Error: Failed to copy bootloader to disk image"
    exit 1
fi

# Copy kernel to disk image
dd if=kernel.bin of=os.img bs=512 seek=1 conv=notrunc 2>/dev/null
if [ $? -ne 0 ]; then
    echo "Error: Failed to copy kernel to disk image"
    exit 1
fi

echo "Build completed successfully!"
echo ""
echo "To run the OS:"
echo "  qemu-system-i386 -fda os.img -m 16"
echo "" 