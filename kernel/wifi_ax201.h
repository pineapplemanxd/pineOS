#ifndef WIFI_AX201_H
#define WIFI_AX201_H

#include "network.h"
#include "pci.h"

// Intel Wi-Fi 6 AX201 Constants
#define INTEL_WIFI_VENDOR_ID        0x8086  // Intel Corporation
#define INTEL_AX201_DEVICE_ID       0x06F0  // Wi-Fi 6 AX201 160MHz
#define INTEL_AX201_DEVICE_ID2      0x34F0  // Wi-Fi 6 AX201 (alternative)
#define INTEL_AX200_DEVICE_ID       0x2723  // Wi-Fi 6 AX200 (similar)
#define INTEL_AX210_DEVICE_ID       0x2725  // Wi-Fi 6E AX210

// Wi-Fi 6 specific constants
#define WIFI6_MAX_CHANNELS          233     // Wi-Fi 6 supports more channels
#define WIFI6_MAX_BANDWIDTH         160     // 160MHz channel width
#define WIFI6_MAX_STREAMS           4       // 4x4 MIMO
#define WIFI6_MAX_SPEED_MBPS        2400    // Maximum theoretical speed

// AX201 Register Offsets (Intel iwlwifi compatible)
#define AX201_CSR_BASE              0x0000
#define AX201_CSR_HW_IF_CONFIG      0x000
#define AX201_CSR_INT_COALESCING    0x004
#define AX201_CSR_INT               0x008
#define AX201_CSR_INT_MASK          0x00C
#define AX201_CSR_FH_INT_STATUS     0x010
#define AX201_CSR_GPIO_IN           0x018
#define AX201_CSR_RESET             0x020
#define AX201_CSR_GP_CNTRL          0x024
#define AX201_CSR_HW_REV            0x028
#define AX201_CSR_EEPROM_REG        0x02C
#define AX201_CSR_EEPROM_GP         0x030
#define AX201_CSR_OTP_GP_REG        0x034
#define AX201_CSR_GIO_REG           0x03C
#define AX201_CSR_GP_UCODE_REG      0x048
#define AX201_CSR_UCODE_DRV_GP1     0x054
#define AX201_CSR_UCODE_DRV_GP2     0x058
#define AX201_CSR_LED_REG           0x094
#define AX201_CSR_DRAM_INT_TBL      0x0A0
#define AX201_CSR_GIO_CHICKEN_BITS  0x100

// Wi-Fi 6 Frame Types
#define WIFI6_FRAME_BEACON          0x80
#define WIFI6_FRAME_PROBE_REQ       0x40
#define WIFI6_FRAME_PROBE_RESP      0x50
#define WIFI6_FRAME_AUTH            0xB0
#define WIFI6_FRAME_ASSOC_REQ       0x00
#define WIFI6_FRAME_ASSOC_RESP      0x10
#define WIFI6_FRAME_DATA            0x08

// Wi-Fi 6 Security Types
#define WIFI6_SECURITY_NONE         0
#define WIFI6_SECURITY_WEP          1
#define WIFI6_SECURITY_WPA          2
#define WIFI6_SECURITY_WPA2         3
#define WIFI6_SECURITY_WPA3         4
#define WIFI6_SECURITY_WPA3_SAE     5

// Wi-Fi 6 Network Structure
typedef struct wifi6_network {
    char ssid[MAX_SSID_LENGTH];
    uint8_t bssid[6];
    uint8_t channel;
    uint8_t bandwidth;          // 20, 40, 80, 160 MHz
    int8_t signal_strength;     // dBm
    uint8_t security_type;
    uint16_t beacon_interval;
    uint32_t capabilities;
    uint8_t wifi6_features;     // OFDMA, MU-MIMO, etc.
    int used;
} wifi6_network_t;

// AX201 Device Structure
typedef struct ax201_device {
    pci_device_t* pci_dev;
    uint32_t mmio_base;
    uint32_t mmio_size;
    mac_address_t mac_addr;
    
    // Firmware and microcode
    uint8_t* firmware_data;
    uint32_t firmware_size;
    int firmware_loaded;
    
    // Wi-Fi 6 specific
    uint8_t current_channel;
    uint8_t current_bandwidth;
    uint8_t tx_power;
    uint8_t antenna_config;
    
    // Connection state
    wifi6_network_t* connected_network;
    uint8_t connection_state;
    uint8_t auth_state;
    
    // Buffers and queues
    void* tx_queue;
    void* rx_queue;
    void* cmd_queue;
    
    int initialized;
    int radio_enabled;
} ax201_device_t;

// Connection states
#define WIFI_STATE_DISCONNECTED     0
#define WIFI_STATE_SCANNING          1
#define WIFI_STATE_CONNECTING        2
#define WIFI_STATE_AUTHENTICATING    3
#define WIFI_STATE_ASSOCIATING       4
#define WIFI_STATE_CONNECTED         5
#define WIFI_STATE_ERROR             6

// Function declarations
int ax201_init(void);
int ax201_detect_device(void);
int ax201_setup_device(pci_device_t* pci_dev);
int ax201_load_firmware(ax201_device_t* dev);
int ax201_enable_radio(ax201_device_t* dev);
int ax201_disable_radio(ax201_device_t* dev);

// Wi-Fi 6 operations
int ax201_scan_networks(wifi6_network_t* networks, int max_networks);
int ax201_connect_network(const char* ssid, const char* password, uint8_t security_type);
int ax201_disconnect_network(void);
int ax201_get_connection_status(void);

// Packet operations
int ax201_send_packet(const void* data, uint32_t len);
int ax201_receive_packet(void* buffer, uint32_t max_len);

// Configuration
int ax201_set_channel(uint8_t channel, uint8_t bandwidth);
int ax201_set_tx_power(uint8_t power_dbm);
int ax201_get_signal_strength(void);
void ax201_read_mac_address(ax201_device_t* dev);

// Register access
uint32_t ax201_read_reg(ax201_device_t* dev, uint32_t reg);
void ax201_write_reg(ax201_device_t* dev, uint32_t reg, uint32_t value);

// Utility functions
ax201_device_t* get_ax201_device(void);
void ax201_show_device_info(void);
void ax201_show_wifi6_capabilities(void);

// Wi-Fi 6 specific features
int ax201_enable_ofdma(void);
int ax201_enable_mu_mimo(void);
int ax201_set_target_wake_time(uint32_t twt_interval);

#endif