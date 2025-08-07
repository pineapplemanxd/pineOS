#!/bin/bash

# Simple USB image creation script for pineOS

echo "Creating simple USB-bootable pineOS image..."

# Build the OS first
make clean
make os.iso

if [ ! -f "os.iso" ]; then
    echo "Error: Failed to build os.iso"
    exit 1
fi

# Create a simple raw disk image (16MB)
USB_IMAGE="pineos_usb.img"
USB_SIZE_MB=16

echo "Creating raw USB image: $USB_IMAGE (${USB_SIZE_MB}MB)"

# Create empty image file
dd if=/dev/zero of=$USB_IMAGE bs=1M count=$USB_SIZE_MB 2>/dev/null

# Install GRUB to the image (this creates a bootable image)
echo "Installing GRUB bootloader..."

# Create temporary directory structure
mkdir -p usb_temp/boot/grub

# Copy kernel and grub config
cp kernel.elf usb_temp/boot/
cp grub.cfg usb_temp/boot/grub/

# Create a simple GRUB installation
# Note: This is a simplified approach that may not work on all systems
echo "Creating bootable image..."

# Use grub-mkrescue to create a hybrid ISO that can be written to USB
grub-mkrescue -o $USB_IMAGE usb_temp

# Clean up
rm -rf usb_temp

echo ""
echo "USB image created: $USB_IMAGE"
echo ""
echo "To write to USB drive on Linux (replace /dev/sdX with your USB device):"
echo "  sudo dd if=$USB_IMAGE of=/dev/sdX bs=1M status=progress"
echo ""
echo "To write to USB drive on Windows:"
echo "  1. Download Rufus from https://rufus.ie/"
echo "  2. Select your USB drive"
echo "  3. Select 'DD Image' mode"
echo "  4. Choose the $USB_IMAGE file"
echo "  5. Click START"
echo ""
echo "To test in QEMU:"
echo "  qemu-system-i386 -drive file=$USB_IMAGE,format=raw -m 32"
echo ""
echo "Your pineOS now includes:"
echo "  - Persistent filesystem support"
echo "  - USB storage detection"
echo "  - Commands: storage, save, load, format"
echo "  - All original filesystem commands"