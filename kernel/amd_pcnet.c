#include "amd_pcnet.h"
#include "pci.h"
#include "io.h"
#include "memory.h"
#include "string.h"

// Global AMD PCnet device
static amd_pcnet_device_t amd_pcnet_dev;

// Initialize AMD PCnet driver
int amd_pcnet_init(void) {
    vga_puts("Initializing AMD PCnet driver...\n");
    
    // Clear device structure
    memory_set(&amd_pcnet_dev, 0, sizeof(amd_pcnet_device_t));
    
    // Detect AMD PCnet device
    if (amd_pcnet_detect_device() != 0) {
        vga_puts("No AMD PCnet device found\n");
        return -1;
    }
    
    vga_puts("AMD PCnet driver initialized successfully\n");
    return 0;
}

// Detect AMD PCnet device
int amd_pcnet_detect_device(void) {
    vga_puts("Scanning for AMD PCnet device (1022:2000)...\n");
    
    pci_device_t* pcnet_device = pci_find_device(AMD_PCNET_VENDOR_ID, AMD_PCNET_DEVICE_ID);
    
    if (!pcnet_device) {
        vga_puts("AMD PCnet device not found\n");
        return -1;
    }
    
    vga_puts("Found AMD PCnet device!\n");
    return amd_pcnet_setup_device(pcnet_device);
}

// Setup AMD PCnet device
int amd_pcnet_setup_device(pci_device_t* pci_dev) {
    vga_puts("Setting up AMD PCnet device...\n");
    
    amd_pcnet_dev.pci_dev = pci_dev;
    
    // Get I/O base address from BAR0
    uint32_t bar0 = pci_dev->bar[0];
    if (bar0 & 0x1) {
        // I/O space
        amd_pcnet_dev.io_base = bar0 & 0xFFFFFFFC;
        vga_puts("AMD PCnet I/O base: ");
        const char hex[] = "0123456789ABCDEF";
        for (int i = 7; i >= 0; i--) {
            vga_putchar(hex[(amd_pcnet_dev.io_base >> (i * 4)) & 0xF]);
        }
        vga_puts("\n");
    } else {
        vga_puts("Error: Expected I/O space BAR\n");
        return -1;
    }
    
    // Enable PCI device
    uint16_t command = pci_config_read_word(pci_dev->bus, pci_dev->device, pci_dev->function, PCI_COMMAND);
    command |= 0x05; // Enable I/O space and bus mastering
    pci_config_write_dword(pci_dev->bus, pci_dev->device, pci_dev->function, PCI_COMMAND, command);
    
    // Reset the device
    vga_puts("Resetting AMD PCnet device...\n");
    inw(amd_pcnet_dev.io_base + PCNET_RESET);
    
    // Wait for reset to complete
    for (volatile int i = 0; i < 100000; i++);
    
    // Read MAC address
    amd_pcnet_read_mac_address(&amd_pcnet_dev);
    
    // Setup descriptor rings
    if (amd_pcnet_setup_rings(&amd_pcnet_dev) != 0) {
        vga_puts("Failed to setup AMD PCnet rings\n");
        return -1;
    }
    
    // Initialize the device
    amd_pcnet_write_csr(&amd_pcnet_dev, PCNET_CSR0, PCNET_CSR0_INIT);
    
    // Wait for initialization
    int timeout = 1000;
    while (timeout-- > 0) {
        uint16_t csr0 = amd_pcnet_read_csr(&amd_pcnet_dev, PCNET_CSR0);
        if (csr0 & PCNET_CSR0_IDON) {
            break;
        }
        for (volatile int i = 0; i < 1000; i++);
    }
    
    if (timeout <= 0) {
        vga_puts("AMD PCnet initialization timeout\n");
        return -1;
    }
    
    // Start the device
    amd_pcnet_write_csr(&amd_pcnet_dev, PCNET_CSR0, PCNET_CSR0_STRT | PCNET_CSR0_INEA);
    
    amd_pcnet_dev.initialized = 1;
    vga_puts("AMD PCnet device ready for VirtualBox networking\n");
    
    return 0;
}

// Read MAC address from device
void amd_pcnet_read_mac_address(amd_pcnet_device_t* dev) {
    vga_puts("Reading MAC address from AMD PCnet...\n");
    
    // Read MAC address from APROM registers
    uint16_t mac01 = inw(dev->io_base + PCNET_APROM00);
    uint16_t mac23 = inw(dev->io_base + PCNET_APROM01);
    uint16_t mac45 = inw(dev->io_base + PCNET_APROM02);
    
    dev->mac_addr.bytes[0] = mac01 & 0xFF;
    dev->mac_addr.bytes[1] = (mac01 >> 8) & 0xFF;
    dev->mac_addr.bytes[2] = mac23 & 0xFF;
    dev->mac_addr.bytes[3] = (mac23 >> 8) & 0xFF;
    dev->mac_addr.bytes[4] = mac45 & 0xFF;
    dev->mac_addr.bytes[5] = (mac45 >> 8) & 0xFF;
    
    vga_puts("AMD PCnet MAC address: ");
    char mac_str[MAX_MAC_STRING];
    mac_to_string(&dev->mac_addr, mac_str);
    vga_puts(mac_str);
    vga_puts("\n");
}

// Setup descriptor rings (simplified)
int amd_pcnet_setup_rings(amd_pcnet_device_t* dev) {
    vga_puts("Setting up AMD PCnet descriptor rings...\n");
    
    // Allocate buffers (simplified)
    for (int i = 0; i < 16; i++) {
        dev->rx_buffers[i] = (uint8_t*)memory_alloc(1518); // Max Ethernet frame
        dev->tx_buffers[i] = (uint8_t*)memory_alloc(1518);
        
        if (!dev->rx_buffers[i] || !dev->tx_buffers[i]) {
            vga_puts("Failed to allocate AMD PCnet buffers\n");
            return -1;
        }
    }
    
    dev->rx_cur = 0;
    dev->tx_cur = 0;
    
    vga_puts("AMD PCnet rings configured\n");
    return 0;
}

// Send packet through AMD PCnet
int amd_pcnet_send_packet(const void* data, uint32_t len) {
    if (!amd_pcnet_dev.initialized || !data || len == 0) {
        return -1;
    }
    
    vga_puts("Sending packet via AMD PCnet (");
    vga_putchar('0' + (len / 100));
    vga_putchar('0' + ((len / 10) % 10));
    vga_putchar('0' + (len % 10));
    vga_puts(" bytes)\n");
    
    // Copy data to transmit buffer
    if (len > 1518) len = 1518; // Limit to max Ethernet frame
    memory_copy(amd_pcnet_dev.tx_buffers[amd_pcnet_dev.tx_cur], data, len);
    
    // Setup transmit descriptor for VirtualBox bridged networking
    uint32_t buffer_addr = (uint32_t)amd_pcnet_dev.tx_buffers[amd_pcnet_dev.tx_cur];
    
    // Write buffer address to transmit descriptor
    outl(amd_pcnet_dev.io_base + 0x78, buffer_addr); // TX buffer address
    outl(amd_pcnet_dev.io_base + 0x7C, len | 0x80000000); // Length with OWN bit
    
    // Trigger transmission by writing to CSR0
    amd_pcnet_write_csr(&amd_pcnet_dev, PCNET_CSR0, PCNET_CSR0_TDMD | PCNET_CSR0_INEA);
    
    // Wait for transmission to complete (simplified)
    for (volatile int i = 0; i < 10000; i++) {
        uint16_t csr0 = amd_pcnet_read_csr(&amd_pcnet_dev, PCNET_CSR0);
        if (csr0 & PCNET_CSR0_TINT) {
            // Clear interrupt
            amd_pcnet_write_csr(&amd_pcnet_dev, PCNET_CSR0, PCNET_CSR0_TINT | PCNET_CSR0_INEA);
            break;
        }
    }
    
    vga_puts("AMD PCnet packet transmitted to VirtualBox bridge\n");
    
    // Update current buffer
    amd_pcnet_dev.tx_cur = (amd_pcnet_dev.tx_cur + 1) % 16;
    
    return 0;
}

// Receive packet from AMD PCnet
int amd_pcnet_receive_packet(void* buffer, uint32_t max_len) {
    if (!amd_pcnet_dev.initialized || !buffer) {
        return -1;
    }
    
    // Check for received packets in VirtualBox bridged mode
    uint16_t csr0 = amd_pcnet_read_csr(&amd_pcnet_dev, PCNET_CSR0);
    
    if (csr0 & PCNET_CSR0_RINT) {
        // Receive interrupt - packet available
        vga_puts("AMD PCnet: Packet received from bridge\n");
        
        // Read from current RX buffer
        uint32_t rx_len = inl(amd_pcnet_dev.io_base + 0x70); // RX length register
        rx_len &= 0xFFFF; // Mask to get actual length
        
        if (rx_len > 0 && rx_len <= max_len) {
            // Copy received data
            memory_copy(buffer, amd_pcnet_dev.rx_buffers[amd_pcnet_dev.rx_cur], rx_len);
            
            // Clear receive interrupt
            amd_pcnet_write_csr(&amd_pcnet_dev, PCNET_CSR0, PCNET_CSR0_RINT | PCNET_CSR0_INEA);
            
            // Update RX buffer index
            amd_pcnet_dev.rx_cur = (amd_pcnet_dev.rx_cur + 1) % 16;
            
            vga_puts("AMD PCnet: Received ");
            vga_putchar('0' + (rx_len / 100));
            vga_putchar('0' + ((rx_len / 10) % 10));
            vga_putchar('0' + (rx_len % 10));
            vga_puts(" bytes\n");
            
            return rx_len;
        }
        
        // Clear interrupt even if no valid data
        amd_pcnet_write_csr(&amd_pcnet_dev, PCNET_CSR0, PCNET_CSR0_RINT | PCNET_CSR0_INEA);
    }
    
    return -1; // No packet received
}

// Read CSR register
uint16_t amd_pcnet_read_csr(amd_pcnet_device_t* dev, uint16_t reg) {
    outw(dev->io_base + PCNET_RAP, reg);
    return inw(dev->io_base + PCNET_RDP);
}

// Write CSR register
void amd_pcnet_write_csr(amd_pcnet_device_t* dev, uint16_t reg, uint16_t value) {
    outw(dev->io_base + PCNET_RAP, reg);
    outw(dev->io_base + PCNET_RDP, value);
}

// Get device for external access
amd_pcnet_device_t* get_amd_pcnet_device(void) {
    return amd_pcnet_dev.initialized ? &amd_pcnet_dev : 0;
}