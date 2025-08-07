#!/bin/bash

echo "Setting up VirtualBox VM for PineOS with Real Networking"
echo "========================================================"

VM_NAME="PineOS"
ISO_PATH="$(pwd)/os.iso"

# Check if VirtualBox is installed
if ! command -v VBoxManage &> /dev/null; then
    echo "Error: VirtualBox is not installed or not in PATH"
    echo "Please install VirtualBox first"
    exit 1
fi

# Check if ISO exists
if [ ! -f "$ISO_PATH" ]; then
    echo "Error: os.iso not found. Please build the OS first with 'make'"
    exit 1
fi

echo "Creating VirtualBox VM: $VM_NAME"

# Delete existing VM if it exists
VBoxManage unregistervm "$VM_NAME" --delete 2>/dev/null || true

# Create new VM
VBoxManage createvm --name "$VM_NAME" --ostype "Other" --register

# Configure VM settings
VBoxManage modifyvm "$VM_NAME" \
    --memory 128 \
    --vram 16 \
    --cpus 1 \
    --boot1 dvd \
    --boot2 none \
    --boot3 none \
    --boot4 none \
    --acpi on \
    --ioapic on \
    --rtcuseutc on \
    --bioslogofadein off \
    --bioslogofadeout off \
    --bioslogodisplaytime 0 \
    --biosbootmenu disabled

# Configure network adapter for bridged networking
echo "Configuring network adapter for real network access..."
VBoxManage modifyvm "$VM_NAME" \
    --nic1 bridged \
    --nictype1 82540EM \
    --bridgeadapter1 "$(VBoxManage list bridgedifs | grep "^Name:" | head -1 | cut -d' ' -f2-)" \
    --cableconnected1 on

# Create and attach DVD drive with our ISO
VBoxManage storagectl "$VM_NAME" --name "IDE Controller" --add ide
VBoxManage storageattach "$VM_NAME" \
    --storagectl "IDE Controller" \
    --port 0 \
    --device 0 \
    --type dvddrive \
    --medium "$ISO_PATH"

echo ""
echo "VirtualBox VM '$VM_NAME' created successfully!"
echo ""
echo "Network Configuration:"
echo "- Adapter Type: Intel E1000 (82540EM)"
echo "- Network Mode: Bridged"
echo "- This allows your OS to get a real IP address on your network"
echo ""
echo "To start the VM:"
echo "  VBoxManage startvm '$VM_NAME'"
echo ""
echo "To start with GUI:"
echo "  VBoxManage startvm '$VM_NAME' --type gui"
echo ""
echo "To start headless (no GUI):"
echo "  VBoxManage startvm '$VM_NAME' --type headless"
echo ""
echo "Once booted, try these commands in your OS:"
echo "  ifconfig           # Show network interfaces"
echo "  dhcp eth0          # Get IP address via DHCP"
echo "  ping 8.8.8.8       # Ping Google DNS"
echo "  ping [your-router-ip]  # Ping your router"
echo ""