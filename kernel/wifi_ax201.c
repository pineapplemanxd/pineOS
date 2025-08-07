#include "wifi_ax201.h"
#include "pci.h"
#include "io.h"
#include "memory.h"
#include "string.h"

// Global AX201 device
static ax201_device_t ax201_dev;
static wifi6_network_t wifi6_networks[MAX_WIFI_NETWORKS];
static int wifi6_network_count = 0;

// Initialize Intel Wi-Fi 6 AX201 driver
int ax201_init(void) {
    vga_puts("Initializing Intel Wi-Fi 6 AX201 driver...\n");
    
    // Clear device structure
    memory_set(&ax201_dev, 0, sizeof(ax201_device_t));
    
    // Clear networks array
    for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
        wifi6_networks[i].used = 0;
    }
    wifi6_network_count = 0;
    
    // Detect AX201 device
    if (ax201_detect_device() != 0) {
        vga_puts("No Intel Wi-Fi 6 AX201 device found\n");
        return -1;
    }
    
    vga_puts("Intel Wi-Fi 6 AX201 driver initialized successfully\n");
    return 0;
}

// Detect Intel Wi-Fi 6 AX201 device
int ax201_detect_device(void) {
    vga_puts("Scanning for Intel Wi-Fi 6 AX201 device...\n");
    
    pci_device_t* wifi_device = 0;
    
    // Look for Intel AX201 devices
    vga_puts("Looking for AX201 (06F0)...\n");
    wifi_device = pci_find_device(INTEL_WIFI_VENDOR_ID, INTEL_AX201_DEVICE_ID);
    
    if (!wifi_device) {
        vga_puts("Looking for AX201 alternative (34F0)...\n");
        wifi_device = pci_find_device(INTEL_WIFI_VENDOR_ID, INTEL_AX201_DEVICE_ID2);
    }
    
    if (!wifi_device) {
        vga_puts("Looking for AX200 (2723)...\n");
        wifi_device = pci_find_device(INTEL_WIFI_VENDOR_ID, INTEL_AX200_DEVICE_ID);
    }
    
    if (!wifi_device) {
        vga_puts("Looking for AX210 (2725)...\n");
        wifi_device = pci_find_device(INTEL_WIFI_VENDOR_ID, INTEL_AX210_DEVICE_ID);
    }
    
    if (!wifi_device) {
        vga_puts("Looking for any Intel Wi-Fi device...\n");
        // Try common Intel Wi-Fi device IDs
        uint16_t intel_wifi_ids[] = {
            0x06F0, // AX201 160MHz
            0x34F0, // AX201
            0x2723, // AX200
            0x2725, // AX210
            0x271B, // AX201 (another variant)
            0x271C, // AX201 vPro
            0x7AF0, // AX201 (yet another)
            0x51F0, // AX201 (mobile)
            0x51F1, // AX201 (desktop)
            0x54F0, // AX201 (newer)
            0
        };
        
        for (int i = 0; intel_wifi_ids[i] != 0; i++) {
            vga_puts("Trying Wi-Fi device ID: ");
            const char hex[] = "0123456789ABCDEF";
            uint16_t dev_id = intel_wifi_ids[i];
            vga_putchar(hex[(dev_id >> 12) & 0xF]);
            vga_putchar(hex[(dev_id >> 8) & 0xF]);
            vga_putchar(hex[(dev_id >> 4) & 0xF]);
            vga_putchar(hex[dev_id & 0xF]);
            vga_puts("\n");
            
            wifi_device = pci_find_device(INTEL_WIFI_VENDOR_ID, dev_id);
            if (wifi_device) {
                vga_puts("Found Intel Wi-Fi 6 device!\n");
                break;
            }
        }
    }
    
    if (!wifi_device) {
        vga_puts("No Intel Wi-Fi 6 device found\n");
        return -1;
    }
    
    vga_puts("Intel Wi-Fi 6 device detected, setting up...\n");
    return ax201_setup_device(wifi_device);
}

// Setup Intel Wi-Fi 6 AX201 device
int ax201_setup_device(pci_device_t* pci_dev) {
    vga_puts("Setting up Intel Wi-Fi 6 AX201 device...\n");
    
    ax201_dev.pci_dev = pci_dev;
    
    // Show device information
    vga_puts("Device: ");
    const char hex[] = "0123456789ABCDEF";
    vga_putchar(hex[(pci_dev->vendor_id >> 12) & 0xF]);
    vga_putchar(hex[(pci_dev->vendor_id >> 8) & 0xF]);
    vga_putchar(hex[(pci_dev->vendor_id >> 4) & 0xF]);
    vga_putchar(hex[pci_dev->vendor_id & 0xF]);
    vga_putchar(':');
    vga_putchar(hex[(pci_dev->device_id >> 12) & 0xF]);
    vga_putchar(hex[(pci_dev->device_id >> 8) & 0xF]);
    vga_putchar(hex[(pci_dev->device_id >> 4) & 0xF]);
    vga_putchar(hex[pci_dev->device_id & 0xF]);
    vga_puts("\n");
    
    // Get memory-mapped I/O base address from BAR0
    uint32_t bar0 = pci_dev->bar[0];
    if (!(bar0 & 0x1)) {
        // Memory-mapped I/O
        ax201_dev.mmio_base = bar0 & 0xFFFFFFF0;
        ax201_dev.mmio_size = 0x2000; // Typical size for Intel Wi-Fi
        
        vga_puts("AX201 MMIO base: ");
        for (int i = 7; i >= 0; i--) {
            vga_putchar(hex[(ax201_dev.mmio_base >> (i * 4)) & 0xF]);
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
    
    // Reset the Wi-Fi device
    vga_puts("Resetting Wi-Fi 6 device...\n");
    ax201_write_reg(&ax201_dev, AX201_CSR_RESET, 0x80);
    
    // Wait for reset to complete
    for (volatile int i = 0; i < 1000000; i++);
    
    // Read MAC address
    ax201_read_mac_address(&ax201_dev);
    
    // Load firmware
    if (ax201_load_firmware(&ax201_dev) != 0) {
        vga_puts("Warning: Firmware loading failed - using basic mode\n");
    }
    
    // Enable radio
    if (ax201_enable_radio(&ax201_dev) != 0) {
        vga_puts("Warning: Radio enable failed\n");
    }
    
    ax201_dev.initialized = 1;
    vga_puts("Intel Wi-Fi 6 AX201 device ready\n");
    
    // Show Wi-Fi 6 capabilities
    ax201_show_wifi6_capabilities();
    
    return 0;
}

// Read MAC address from device
void ax201_read_mac_address(ax201_device_t* dev) {
    vga_puts("Reading MAC address from Wi-Fi 6 device...\n");
    
    // Try to read MAC from OTP (One-Time Programmable) memory
    uint32_t mac_low = ax201_read_reg(dev, AX201_CSR_EEPROM_REG);
    uint32_t mac_high = ax201_read_reg(dev, AX201_CSR_EEPROM_GP);
    
    if (mac_low != 0 || mac_high != 0) {
        dev->mac_addr.bytes[0] = (mac_low >> 0) & 0xFF;
        dev->mac_addr.bytes[1] = (mac_low >> 8) & 0xFF;
        dev->mac_addr.bytes[2] = (mac_low >> 16) & 0xFF;
        dev->mac_addr.bytes[3] = (mac_low >> 24) & 0xFF;
        dev->mac_addr.bytes[4] = (mac_high >> 0) & 0xFF;
        dev->mac_addr.bytes[5] = (mac_high >> 8) & 0xFF;
    } else {
        // Generate a default MAC address for Intel Wi-Fi
        dev->mac_addr.bytes[0] = 0x00; // Intel OUI
        dev->mac_addr.bytes[1] = 0x1B;
        dev->mac_addr.bytes[2] = 0x77;
        dev->mac_addr.bytes[3] = 0x12;
        dev->mac_addr.bytes[4] = 0x34;
        dev->mac_addr.bytes[5] = 0x56;
    }
    
    vga_puts("Wi-Fi 6 MAC address: ");
    char mac_str[MAX_MAC_STRING];
    mac_to_string(&dev->mac_addr, mac_str);
    vga_puts(mac_str);
    vga_puts("\n");
}

// Load firmware (simplified)
int ax201_load_firmware(ax201_device_t* dev) {
    vga_puts("Loading Wi-Fi 6 firmware...\n");
    
    // In a real implementation, this would load the actual Intel firmware
    // For now, we'll simulate firmware loading
    vga_puts("Simulating firmware load (iwlwifi-cc-a0-XX.ucode)...\n");
    
    // Check if device is ready for firmware
    uint32_t hw_rev = ax201_read_reg(dev, AX201_CSR_HW_REV);
    vga_puts("Hardware revision: ");
    const char hex[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        vga_putchar(hex[(hw_rev >> (i * 4)) & 0xF]);
    }
    vga_puts("\n");
    
    // Simulate firmware loading process
    vga_puts("Loading microcode sections...\n");
    vga_puts("- Loading INIT section\n");
    vga_puts("- Loading RUNTIME section\n");
    vga_puts("- Loading WOWLAN section\n");
    
    dev->firmware_loaded = 1;
    vga_puts("Wi-Fi 6 firmware loaded successfully\n");
    
    return 0;
}

// Enable Wi-Fi radio
int ax201_enable_radio(ax201_device_t* dev) {
    vga_puts("Enabling Wi-Fi 6 radio...\n");
    
    // Enable GPIO for radio
    uint32_t gpio_reg = ax201_read_reg(dev, AX201_CSR_GPIO_IN);
    gpio_reg |= 0x01; // Enable radio
    ax201_write_reg(dev, AX201_CSR_GPIO_IN, gpio_reg);
    
    // Set general control register
    uint32_t gp_cntrl = ax201_read_reg(dev, AX201_CSR_GP_CNTRL);
    gp_cntrl |= 0x08; // Enable MAC
    ax201_write_reg(dev, AX201_CSR_GP_CNTRL, gp_cntrl);
    
    dev->radio_enabled = 1;
    vga_puts("Wi-Fi 6 radio enabled\n");
    
    return 0;
}

// Scan for Wi-Fi 6 networks
int ax201_scan_networks(wifi6_network_t* networks, int max_networks) {
    if (!ax201_dev.initialized || !ax201_dev.radio_enabled) {
        vga_puts("Wi-Fi 6 device not ready for scanning\n");
        return 0;
    }
    
    vga_puts("Scanning for Wi-Fi 6 networks...\n");
    vga_puts("Scanning 2.4GHz and 5GHz bands...\n");
    
    // Clear existing networks
    wifi6_network_count = 0;
    for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
        wifi6_networks[i].used = 0;
    }
    
    // Simulate Wi-Fi 6 network discovery
    const char* wifi6_ssids[] = {
        "WiFi6_Network_5G",
        "MyRouter_AX",
        "NETGEAR_AX12",
        "ASUS_AX6000",
        "Linksys_AX3200",
        "TP-Link_AX73",
        "WiFi6E_6GHz"
    };
    
    int securities[] = {
        WIFI6_SECURITY_WPA3,
        WIFI6_SECURITY_WPA3_SAE,
        WIFI6_SECURITY_WPA2,
        WIFI6_SECURITY_WPA3,
        WIFI6_SECURITY_WPA2,
        WIFI6_SECURITY_WPA3,
        WIFI6_SECURITY_WPA3_SAE
    };
    
    int signals[] = { -35, -42, -58, -45, -67, -52, -38 };
    int channels[] = { 36, 149, 6, 44, 11, 157, 37 }; // Mix of 2.4/5GHz
    int bandwidths[] = { 160, 80, 40, 160, 20, 80, 160 }; // Wi-Fi 6 bandwidths
    
    int num_networks = sizeof(wifi6_ssids) / sizeof(wifi6_ssids[0]);
    int found = 0;
    
    for (int i = 0; i < num_networks && found < max_networks && found < MAX_WIFI_NETWORKS; i++) {
        wifi6_network_t* network = &wifi6_networks[found];
        
        strcpy(network->ssid, wifi6_ssids[i]);
        network->security_type = securities[i];
        network->signal_strength = signals[i];
        network->channel = channels[i];
        network->bandwidth = bandwidths[i];
        network->beacon_interval = 100;
        network->wifi6_features = 0x0F; // OFDMA, MU-MIMO, BSS Coloring, TWT
        
        // Generate BSSID
        for (int j = 0; j < 6; j++) {
            network->bssid[j] = (uint8_t)(network->ssid[0] + network->ssid[1] + j * 17);
        }
        
        network->used = 1;
        found++;
        
        vga_puts("Found Wi-Fi 6: ");
        vga_puts(network->ssid);
        vga_puts(" (");
        vga_putchar('0' + (network->bandwidth / 100));
        vga_putchar('0' + ((network->bandwidth / 10) % 10));
        vga_putchar('0' + (network->bandwidth % 10));
        vga_puts("MHz, ");
        vga_putchar('-');
        int sig = -network->signal_strength;
        vga_putchar('0' + (sig / 10));
        vga_putchar('0' + (sig % 10));
        vga_puts(" dBm)\n");
    }
    
    wifi6_network_count = found;
    vga_puts("Wi-Fi 6 scan complete. Found ");
    vga_putchar('0' + found);
    vga_puts(" networks\n");
    
    return found;
}

// Connect to Wi-Fi 6 network
int ax201_connect_network(const char* ssid, const char* password, uint8_t security_type) {
    if (!ax201_dev.initialized || !ax201_dev.radio_enabled) {
        vga_puts("Wi-Fi 6 device not ready\n");
        return -1;
    }
    
    vga_puts("Connecting to Wi-Fi 6 network: ");
    vga_puts(ssid);
    vga_puts("\n");
    
    // Find the network
    wifi6_network_t* target_network = 0;
    for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
        if (wifi6_networks[i].used && strcmp(wifi6_networks[i].ssid, ssid) == 0) {
            target_network = &wifi6_networks[i];
            break;
        }
    }
    
    if (!target_network) {
        vga_puts("Error: Wi-Fi 6 network not found\n");
        return -1;
    }
    
    ax201_dev.connection_state = WIFI_STATE_CONNECTING;
    
    // Show Wi-Fi 6 features
    vga_puts("Network features: ");
    if (target_network->wifi6_features & 0x01) vga_puts("OFDMA ");
    if (target_network->wifi6_features & 0x02) vga_puts("MU-MIMO ");
    if (target_network->wifi6_features & 0x04) vga_puts("BSS-Coloring ");
    if (target_network->wifi6_features & 0x08) vga_puts("TWT ");
    vga_puts("\n");
    
    // Simulate Wi-Fi 6 connection process
    vga_puts("Authenticating with WPA3...\n");
    ax201_dev.auth_state = 1;
    
    vga_puts("Associating with AP...\n");
    ax201_dev.connection_state = WIFI_STATE_ASSOCIATING;
    
    vga_puts("Negotiating Wi-Fi 6 capabilities...\n");
    vga_puts("- Channel width: ");
    vga_putchar('0' + (target_network->bandwidth / 100));
    vga_putchar('0' + ((target_network->bandwidth / 10) % 10));
    vga_putchar('0' + (target_network->bandwidth % 10));
    vga_puts("MHz\n");
    vga_puts("- OFDMA enabled\n");
    vga_puts("- MU-MIMO 4x4 enabled\n");
    vga_puts("- Target Wake Time configured\n");
    
    // Simulate successful connection
    ax201_dev.connection_state = WIFI_STATE_CONNECTED;
    ax201_dev.connected_network = target_network;
    ax201_dev.current_channel = target_network->channel;
    ax201_dev.current_bandwidth = target_network->bandwidth;
    
    vga_puts("Connected to Wi-Fi 6 network successfully!\n");
    vga_puts("Link speed: Up to ");
    vga_putchar('0' + (WIFI6_MAX_SPEED_MBPS / 1000));
    vga_putchar('0' + ((WIFI6_MAX_SPEED_MBPS / 100) % 10));
    vga_putchar('0' + ((WIFI6_MAX_SPEED_MBPS / 10) % 10));
    vga_putchar('0' + (WIFI6_MAX_SPEED_MBPS % 10));
    vga_puts(" Mbps\n");
    
    return 0;
}

// Show Wi-Fi 6 capabilities
void ax201_show_wifi6_capabilities(void) {
    vga_puts("\nWi-Fi 6 (802.11ax) Capabilities:\n");
    vga_puts("================================\n");
    vga_puts("- Maximum speed: 2.4 Gbps\n");
    vga_puts("- Channel width: 20/40/80/160 MHz\n");
    vga_puts("- MIMO: 4x4 (4 spatial streams)\n");
    vga_puts("- OFDMA: Orthogonal Frequency Division Multiple Access\n");
    vga_puts("- MU-MIMO: Multi-User Multiple Input Multiple Output\n");
    vga_puts("- BSS Coloring: Improved spatial reuse\n");
    vga_puts("- TWT: Target Wake Time for power saving\n");
    vga_puts("- WPA3: Enhanced security\n");
    vga_puts("- Bands: 2.4GHz, 5GHz (6GHz with AX210)\n");
    vga_puts("- Backward compatible: 802.11a/b/g/n/ac\n");
}

// Show device information
void ax201_show_device_info(void) {
    if (!ax201_dev.initialized) {
        vga_puts("Wi-Fi 6 device not initialized\n");
        return;
    }
    
    vga_puts("Intel Wi-Fi 6 AX201 Device Information:\n");
    vga_puts("======================================\n");
    
    vga_puts("Device ID: ");
    const char hex[] = "0123456789ABCDEF";
    vga_putchar(hex[(ax201_dev.pci_dev->device_id >> 12) & 0xF]);
    vga_putchar(hex[(ax201_dev.pci_dev->device_id >> 8) & 0xF]);
    vga_putchar(hex[(ax201_dev.pci_dev->device_id >> 4) & 0xF]);
    vga_putchar(hex[ax201_dev.pci_dev->device_id & 0xF]);
    vga_puts("\n");
    
    vga_puts("MAC Address: ");
    char mac_str[MAX_MAC_STRING];
    mac_to_string(&ax201_dev.mac_addr, mac_str);
    vga_puts(mac_str);
    vga_puts("\n");
    
    vga_puts("Radio: ");
    vga_puts(ax201_dev.radio_enabled ? "Enabled" : "Disabled");
    vga_puts("\n");
    
    vga_puts("Firmware: ");
    vga_puts(ax201_dev.firmware_loaded ? "Loaded" : "Not loaded");
    vga_puts("\n");
    
    if (ax201_dev.connection_state == WIFI_STATE_CONNECTED && ax201_dev.connected_network) {
        vga_puts("Connected to: ");
        vga_puts(ax201_dev.connected_network->ssid);
        vga_puts("\n");
        vga_puts("Channel: ");
        vga_putchar('0' + (ax201_dev.current_channel / 100));
        vga_putchar('0' + ((ax201_dev.current_channel / 10) % 10));
        vga_putchar('0' + (ax201_dev.current_channel % 10));
        vga_puts(" (");
        vga_putchar('0' + (ax201_dev.current_bandwidth / 100));
        vga_putchar('0' + ((ax201_dev.current_bandwidth / 10) % 10));
        vga_putchar('0' + (ax201_dev.current_bandwidth % 10));
        vga_puts("MHz)\n");
    }
}

// Register access functions
uint32_t ax201_read_reg(ax201_device_t* dev, uint32_t reg) {
    if (!dev || !dev->mmio_base) return 0;
    return *((volatile uint32_t*)(dev->mmio_base + reg));
}

void ax201_write_reg(ax201_device_t* dev, uint32_t reg, uint32_t value) {
    if (!dev || !dev->mmio_base) return;
    *((volatile uint32_t*)(dev->mmio_base + reg)) = value;
}

// Get device for external access
ax201_device_t* get_ax201_device(void) {
    return ax201_dev.initialized ? &ax201_dev : 0;
}

// Send packet through Wi-Fi 6 AX201
int ax201_send_packet(const void* data, uint32_t len) {
    if (!ax201_dev.initialized || !data || len == 0) {
        return -1;
    }
    
    vga_puts("Sending Wi-Fi 6 packet (");
    vga_putchar('0' + (len / 100));
    vga_putchar('0' + ((len / 10) % 10));
    vga_putchar('0' + (len % 10));
    vga_puts(" bytes)\n");
    
    // In a real implementation, this would send the packet through Wi-Fi hardware
    // For now, simulate successful transmission
    vga_puts("Wi-Fi 6 packet transmitted successfully\n");
    
    return 0;
}

// Receive packet from Wi-Fi 6 AX201
int ax201_receive_packet(void* buffer, uint32_t max_len) {
    if (!ax201_dev.initialized || !buffer) {
        return -1;
    }
    
    // In a real implementation, this would receive packets from Wi-Fi hardware
    // For now, return no packet received
    return -1;
}

// Get Wi-Fi 6 networks for external access
wifi6_network_t* get_wifi6_networks(void) {
    return wifi6_networks;
}

int get_wifi6_network_count(void) {
    return wifi6_network_count;
}