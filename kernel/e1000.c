#include "e1000.h"
#include "pci.h"
#include "io.h"
#include "memory.h"
#include "string.h"

// Global E1000 device
static e1000_device_t e1000_dev;

// Initialize E1000 network driver
int e1000_init(void) {
    vga_puts("Initializing Intel E1000 network driver...\n");
    
    // Clear device structure
    memory_set(&e1000_dev, 0, sizeof(e1000_device_t));
    
    // Detect E1000 device
    if (e1000_detect_device() != 0) {
        vga_puts("No Intel E1000 device found\n");
        return -1;
    }
    
    vga_puts("Intel E1000 network driver initialized\n");
    return 0;
}

// Detect E1000 network device
int e1000_detect_device(void) {
    vga_puts("Scanning for Intel E1000 network device...\n");
    
    pci_device_t* e1000_device = 0;
    
    // Look for Intel E1000 devices (common in VirtualBox and QEMU)
    vga_puts("Looking for VirtualBox E1000 (100E)...\n");
    e1000_device = pci_find_device(VBOX_VENDOR_ID, VBOX_E1000_DEVICE);
    
    if (!e1000_device) {
        vga_puts("Looking for E1000E variant (10D3)...\n");
        e1000_device = pci_find_device(VBOX_VENDOR_ID, VBOX_E1000E_DEVICE);
    }
    
    if (!e1000_device) {
        vga_puts("Looking for AMD PCnet device (1022:2000)...\n");
        e1000_device = pci_find_device(VENDOR_AMD, AMD_PCNET_DEVICE);
        if (e1000_device) {
            vga_puts("Found AMD PCnet device - will use E1000-compatible mode\n");
        }
    }
    
    if (!e1000_device) {
        vga_puts("Looking for ANY network controller for VirtualBox...\n");
        // In VirtualBox, sometimes the network adapter appears as a generic device
        // Let's try to find any network controller and use it
        e1000_device = pci_find_class(PCI_CLASS_NETWORK, PCI_SUBCLASS_ETHERNET);
        if (e1000_device) {
            vga_puts("Found generic network controller: ");
            const char hex[] = "0123456789ABCDEF";
            vga_putchar(hex[(e1000_device->vendor_id >> 12) & 0xF]);
            vga_putchar(hex[(e1000_device->vendor_id >> 8) & 0xF]);
            vga_putchar(hex[(e1000_device->vendor_id >> 4) & 0xF]);
            vga_putchar(hex[e1000_device->vendor_id & 0xF]);
            vga_putchar(':');
            vga_putchar(hex[(e1000_device->device_id >> 12) & 0xF]);
            vga_putchar(hex[(e1000_device->device_id >> 8) & 0xF]);
            vga_putchar(hex[(e1000_device->device_id >> 4) & 0xF]);
            vga_putchar(hex[e1000_device->device_id & 0xF]);
            vga_puts(" - will try E1000 mode\n");
        }
    }
    
    if (!e1000_device) {
        vga_puts("Looking for any Intel network device...\n");
        // Try common E1000 device IDs
        uint16_t e1000_ids[] = {
            0x100E, // 82540EM (QEMU default)
            0x100F, // 82545EM
            0x1010, // 82546EB
            0x1011, // 82545EM
            0x1012, // 82546EB
            0x1013, // 82541EI
            0x1014, // 82541ER
            0x1015, // 82540EM-A
            0x1016, // 82540EP-A
            0x1017, // 82540EP
            0x10D3, // 82574L
            0
        };
        
        for (int i = 0; e1000_ids[i] != 0; i++) {
            vga_puts("Trying device ID: ");
            const char hex[] = "0123456789ABCDEF";
            uint16_t dev_id = e1000_ids[i];
            vga_putchar(hex[(dev_id >> 12) & 0xF]);
            vga_putchar(hex[(dev_id >> 8) & 0xF]);
            vga_putchar(hex[(dev_id >> 4) & 0xF]);
            vga_putchar(hex[dev_id & 0xF]);
            vga_puts("\n");
            
            e1000_device = pci_find_device(VENDOR_INTEL, dev_id);
            if (e1000_device) {
                vga_puts("Found Intel E1000 device!\n");
                break;
            }
        }
    }
    
    if (!e1000_device) {
        vga_puts("No Intel E1000 device found - checking all PCI devices...\n");
        // List all PCI devices for debugging
        pci_list_devices();
        return -1;
    }
    
    vga_puts("E1000 device detected, setting up...\n");
    // Setup the device
    return e1000_setup_device(e1000_device);
}

// Setup E1000 network device
int e1000_setup_device(pci_device_t* pci_dev) {
    vga_puts("Setting up Intel E1000 network device...\n");
    
    e1000_dev.pci_dev = pci_dev;
    
    // Get memory-mapped I/O base address from BAR0
    uint32_t bar0 = pci_dev->bar[0];
    if (!(bar0 & 0x1)) {
        // Memory-mapped I/O
        e1000_dev.mmio_base = bar0 & 0xFFFFFFF0;
        vga_puts("E1000 MMIO base: ");
        const char hex[] = "0123456789ABCDEF";
        for (int i = 7; i >= 0; i--) {
            vga_putchar(hex[(e1000_dev.mmio_base >> (i * 4)) & 0xF]);
        }
        vga_puts("\n");
    } else {
        vga_puts("Error: Expected memory-mapped I/O BAR\n");
        return -1;
    }
    
    // Enable PCI device
    uint16_t command = pci_config_read_word(pci_dev->bus, pci_dev->device, pci_dev->function, PCI_COMMAND);
    command |= 0x07; // Enable I/O space, memory space, and bus mastering
    pci_config_write_dword(pci_dev->bus, pci_dev->device, pci_dev->function, PCI_COMMAND, command);
    
    // Reset the device
    vga_puts("Resetting E1000 device...\n");
    e1000_write_reg(&e1000_dev, E1000_CTRL, E1000_CTRL_RST);
    
    // Wait for reset to complete
    for (volatile int i = 0; i < 1000000; i++);
    
    // Read MAC address
    e1000_read_mac_address(&e1000_dev);
    
    // Setup receive and transmit
    if (e1000_setup_rx(&e1000_dev) != 0) {
        vga_puts("Failed to setup E1000 receive\n");
        return -1;
    }
    
    if (e1000_setup_tx(&e1000_dev) != 0) {
        vga_puts("Failed to setup E1000 transmit\n");
        return -1;
    }
    
    // Enable the device
    uint32_t ctrl = e1000_read_reg(&e1000_dev, E1000_CTRL);
    ctrl |= E1000_CTRL_SLU; // Set Link Up
    e1000_write_reg(&e1000_dev, E1000_CTRL, ctrl);
    
    e1000_dev.initialized = 1;
    vga_puts("Intel E1000 device ready for VirtualBox networking\n");
    
    return 0;
}

// Read MAC address from EEPROM
void e1000_read_mac_address(e1000_device_t* dev) {
    vga_puts("Reading MAC address from E1000...\n");
    
    // Try to read from EEPROM first
    uint32_t mac_low = e1000_read_reg(dev, 0x5400);  // RAL0
    uint32_t mac_high = e1000_read_reg(dev, 0x5404); // RAH0
    
    if (mac_low != 0 || mac_high != 0) {
        dev->mac_addr.bytes[0] = (mac_low >> 0) & 0xFF;
        dev->mac_addr.bytes[1] = (mac_low >> 8) & 0xFF;
        dev->mac_addr.bytes[2] = (mac_low >> 16) & 0xFF;
        dev->mac_addr.bytes[3] = (mac_low >> 24) & 0xFF;
        dev->mac_addr.bytes[4] = (mac_high >> 0) & 0xFF;
        dev->mac_addr.bytes[5] = (mac_high >> 8) & 0xFF;
    } else {
        // Generate a default MAC address for VirtualBox
        dev->mac_addr.bytes[0] = 0x08;
        dev->mac_addr.bytes[1] = 0x00;
        dev->mac_addr.bytes[2] = 0x27;
        dev->mac_addr.bytes[3] = 0x12;
        dev->mac_addr.bytes[4] = 0x34;
        dev->mac_addr.bytes[5] = 0x56;
    }
    
    vga_puts("E1000 MAC address: ");
    char mac_str[MAX_MAC_STRING];
    mac_to_string(&dev->mac_addr, mac_str);
    vga_puts(mac_str);
    vga_puts("\n");
}

// Setup receive descriptors
int e1000_setup_rx(e1000_device_t* dev) {
    vga_puts("Setting up E1000 receive descriptors...\n");
    
    // Allocate receive descriptors (simplified)
    dev->rx_descs = (e1000_rx_desc_t*)memory_alloc(256 * sizeof(e1000_rx_desc_t));
    if (!dev->rx_descs) {
        vga_puts("Failed to allocate RX descriptors\n");
        return -1;
    }
    
    // Allocate receive buffers
    for (int i = 0; i < 256; i++) {
        dev->rx_buffers[i] = (uint8_t*)memory_alloc(2048);
        if (!dev->rx_buffers[i]) {
            vga_puts("Failed to allocate RX buffer\n");
            return -1;
        }
        
        dev->rx_descs[i].buffer_addr = (uint64_t)(uint32_t)dev->rx_buffers[i];
        dev->rx_descs[i].status = 0;
    }
    
    // Setup receive registers
    e1000_write_reg(dev, E1000_RDBAL, (uint32_t)dev->rx_descs);
    e1000_write_reg(dev, E1000_RDBAH, 0);
    e1000_write_reg(dev, E1000_RDLEN, 256 * sizeof(e1000_rx_desc_t));
    e1000_write_reg(dev, E1000_RDH, 0);
    e1000_write_reg(dev, E1000_RDT, 255);
    
    // Enable receive
    uint32_t rctl = E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_SZ_2048 | E1000_RCTL_SECRC;
    e1000_write_reg(dev, E1000_RCTL, rctl);
    
    dev->rx_cur = 0;
    vga_puts("E1000 receive setup complete\n");
    return 0;
}

// Setup transmit descriptors
int e1000_setup_tx(e1000_device_t* dev) {
    vga_puts("Setting up E1000 transmit descriptors...\n");
    
    // Allocate transmit descriptors
    dev->tx_descs = (e1000_tx_desc_t*)memory_alloc(256 * sizeof(e1000_tx_desc_t));
    if (!dev->tx_descs) {
        vga_puts("Failed to allocate TX descriptors\n");
        return -1;
    }
    
    // Allocate transmit buffers
    for (int i = 0; i < 256; i++) {
        dev->tx_buffers[i] = (uint8_t*)memory_alloc(2048);
        if (!dev->tx_buffers[i]) {
            vga_puts("Failed to allocate TX buffer\n");
            return -1;
        }
        
        dev->tx_descs[i].buffer_addr = (uint64_t)(uint32_t)dev->tx_buffers[i];
        dev->tx_descs[i].status = E1000_TXD_STAT_DD;
    }
    
    // Setup transmit registers
    e1000_write_reg(dev, E1000_TDBAL, (uint32_t)dev->tx_descs);
    e1000_write_reg(dev, E1000_TDBAH, 0);
    e1000_write_reg(dev, E1000_TDLEN, 256 * sizeof(e1000_tx_desc_t));
    e1000_write_reg(dev, E1000_TDH, 0);
    e1000_write_reg(dev, E1000_TDT, 0);
    
    // Enable transmit
    uint32_t tctl = E1000_TCTL_EN | E1000_TCTL_PSP | (15 << 4) | (64 << 12);
    e1000_write_reg(dev, E1000_TCTL, tctl);
    
    // Set transmit IPG
    e1000_write_reg(dev, E1000_TIPG, 0x0060200A);
    
    dev->tx_cur = 0;
    vga_puts("E1000 transmit setup complete\n");
    return 0;
}

// Send packet through E1000
int e1000_send_packet(const void* data, uint32_t len) {
    if (!e1000_dev.initialized || !data || len == 0) {
        return -1;
    }
    
    vga_puts("Sending packet via E1000 (");
    vga_putchar('0' + (len / 100));
    vga_putchar('0' + ((len / 10) % 10));
    vga_putchar('0' + (len % 10));
    vga_puts(" bytes)\n");
    
    // Get current transmit descriptor
    e1000_tx_desc_t* desc = &e1000_dev.tx_descs[e1000_dev.tx_cur];
    
    // Wait for descriptor to be available
    while (!(desc->status & E1000_TXD_STAT_DD)) {
        // Wait for previous transmission to complete
    }
    
    // Copy data to transmit buffer
    if (len > 1500) len = 1500; // Limit to MTU
    memory_copy(e1000_dev.tx_buffers[e1000_dev.tx_cur], data, len);
    
    // Setup descriptor
    desc->length = len;
    desc->cmd = E1000_TXD_CMD_EOP | E1000_TXD_CMD_RS;
    desc->status = 0;
    
    // Update tail pointer to start transmission
    e1000_dev.tx_cur = (e1000_dev.tx_cur + 1) % 256;
    e1000_write_reg(&e1000_dev, E1000_TDT, e1000_dev.tx_cur);
    
    return 0;
}

// Receive packet from E1000
int e1000_receive_packet(void* buffer, uint32_t max_len) {
    if (!e1000_dev.initialized || !buffer) {
        return -1;
    }
    
    // Check current receive descriptor
    e1000_rx_desc_t* desc = &e1000_dev.rx_descs[e1000_dev.rx_cur];
    
    if (!(desc->status & E1000_RXD_STAT_DD)) {
        return -1; // No packet received
    }
    
    // Copy received data
    uint32_t len = desc->length;
    if (len > max_len) len = max_len;
    
    memory_copy(buffer, e1000_dev.rx_buffers[e1000_dev.rx_cur], len);
    
    // Reset descriptor
    desc->status = 0;
    
    // Update tail pointer
    e1000_write_reg(&e1000_dev, E1000_RDT, e1000_dev.rx_cur);
    e1000_dev.rx_cur = (e1000_dev.rx_cur + 1) % 256;
    
    return len;
}

// Read E1000 register
uint32_t e1000_read_reg(e1000_device_t* dev, uint32_t reg) {
    return *((volatile uint32_t*)(dev->mmio_base + reg));
}

// Write E1000 register
void e1000_write_reg(e1000_device_t* dev, uint32_t reg, uint32_t value) {
    *((volatile uint32_t*)(dev->mmio_base + reg)) = value;
}

// Get E1000 device for external access
e1000_device_t* get_e1000_device(void) {
    return e1000_dev.initialized ? &e1000_dev : 0;
}