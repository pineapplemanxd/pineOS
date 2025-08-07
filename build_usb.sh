#!/bin/bash

# Build script for creating USB-bootable pineOS image

echo "Building pineOS for USB boot..."

# Clean previous builds
make clean

# Build the OS
make os.iso

if [ ! -f "os.iso" ]; then
    echo "Error: Failed to build os.iso"
    exit 1
fi

# Create USB image (16MB)
USB_IMAGE="pineos_usb.img"
USB_SIZE_MB=16

echo "Creating USB image: $USB_IMAGE (${USB_SIZE_MB}MB)"

# Create empty image file
dd if=/dev/zero of=$USB_IMAGE bs=1M count=$USB_SIZE_MB 2>/dev/null

# Create partition table
echo "Creating partition table..."
parted -s $USB_IMAGE mklabel msdos
parted -s $USB_IMAGE mkpart primary fat32 1MiB 100%
parted -s $USB_IMAGE set 1 boot on

# Setup loop device for the image
LOOP_DEVICE=$(losetup -f)
if [ -z "$LOOP_DEVICE" ]; then
    echo "Error: No available loop devices"
    exit 1
fi

echo "Using loop device: $LOOP_DEVICE"
sudo losetup $LOOP_DEVICE $USB_IMAGE

# Setup partition loop device
PART_DEVICE="${LOOP_DEVICE}p1"
sudo partprobe $LOOP_DEVICE

# Format the partition as FAT32
echo "Formatting partition as FAT32..."
sudo mkfs.fat -F32 $PART_DEVICE

# Create temporary mount point
MOUNT_POINT="/tmp/pineos_usb_mount"
sudo mkdir -p $MOUNT_POINT

# Mount the partition
sudo mount $PART_DEVICE $MOUNT_POINT

# Install GRUB to the USB image
echo "Installing GRUB bootloader..."
sudo grub-install --target=i386-pc --boot-directory=$MOUNT_POINT/boot $LOOP_DEVICE

# Copy kernel and GRUB config
sudo mkdir -p $MOUNT_POINT/boot/grub
sudo cp kernel.elf $MOUNT_POINT/boot/
sudo cp grub.cfg $MOUNT_POINT/boot/grub/

# Create a simple autorun file for identification
echo "pineOS USB Boot Drive" | sudo tee $MOUNT_POINT/README.txt > /dev/null

# Unmount and cleanup
sudo umount $MOUNT_POINT
sudo rmdir $MOUNT_POINT
sudo losetup -d $LOOP_DEVICE

echo "USB image created successfully: $USB_IMAGE"
echo ""
echo "To write to USB drive (replace /dev/sdX with your USB device):"
echo "  sudo dd if=$USB_IMAGE of=/dev/sdX bs=1M status=progress"
echo ""
echo "WARNING: This will erase all data on the target USB drive!"
echo ""
echo "To test in QEMU:"
echo "  qemu-system-i386 -drive file=$USB_IMAGE,format=raw -m 32"