#!/bin/bash

echo "Building Simple OS with custom bootloader..."

# Clean previous builds
rm -f *.bin *.img kernel/*.o

# Build bootloader
echo "Building bootloader..."
nasm -f bin -o bootloader.bin boot/boot.asm

# Build kernel
echo "Building kernel..."
gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/kernel.o kernel/kernel.c
gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/io.o kernel/io.c
gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/memory.o kernel/memory.c
gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/process.o kernel/process.c
gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/filesystem.o kernel/filesystem.c
gcc -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-pic -mno-red-zone -c -o kernel/string.o kernel/string.c

# Link kernel
echo "Linking kernel..."
ld -m elf_i386 -T linker.ld -o kernel.elf kernel/kernel.o kernel/io.o kernel/memory.o kernel/process.o kernel/filesystem.o kernel/string.o

# Convert to binary
echo "Converting to binary..."
objcopy -O binary kernel.elf kernel.bin

# Create disk image
echo "Creating disk image..."
dd if=/dev/zero of=os.img bs=512 count=2880
dd if=bootloader.bin of=os.img conv=notrunc
dd if=kernel.bin of=os.img bs=512 seek=1 conv=notrunc

echo "Build completed successfully!"
echo "To run: qemu-system-i386 -fda os.img -m 16" 