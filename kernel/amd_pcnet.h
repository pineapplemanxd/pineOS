#ifndef AMD_PCNET_H
#define AMD_PCNET_H

#include "network.h"
#include "pci.h"

// AMD PCnet Constants
#define AMD_PCNET_VENDOR_ID     0x1022  // AMD
#define AMD_PCNET_DEVICE_ID     0x2000  // PCnet (your device!)

// AMD PCnet Register Offsets (I/O based)
#define PCNET_APROM00           0x00    // MAC address bytes 0-1
#define PCNET_APROM01           0x01    // MAC address bytes 2-3
#define PCNET_APROM02           0x02    // MAC address bytes 4-5
#define PCNET_RDP               0x10    // Register Data Port
#define PCNET_RAP               0x12    // Register Address Port
#define PCNET_RESET             0x14    // Reset register
#define PCNET_BDP               0x16    // Bus Configuration Register Data Port

// AMD PCnet CSR (Control and Status Registers)
#define PCNET_CSR0              0x00    // Control and Status
#define PCNET_CSR1              0x01    // Initialization Block Address Low
#define PCNET_CSR2              0x02    // Initialization Block Address High
#define PCNET_CSR3              0x03    // Interrupt Masks and Deferral Control
#define PCNET_CSR4              0x04    // Test and Features Control
#define PCNET_CSR5              0x05    // Extended Control and Interrupt 1
#define PCNET_CSR15             0x0F    // Mode Register

// CSR0 bits
#define PCNET_CSR0_INIT         0x0001  // Initialize
#define PCNET_CSR0_STRT         0x0002  // Start
#define PCNET_CSR0_STOP         0x0004  // Stop
#define PCNET_CSR0_TDMD         0x0008  // Transmit Demand
#define PCNET_CSR0_TXON         0x0010  // Transmitter On
#define PCNET_CSR0_RXON         0x0020  // Receiver On
#define PCNET_CSR0_INEA         0x0040  // Interrupt Enable
#define PCNET_CSR0_RINT         0x0400  // Receive Interrupt
#define PCNET_CSR0_TINT         0x0200  // Transmit Interrupt
#define PCNET_CSR0_IDON         0x0100  // Initialization Done

// AMD PCnet Device Structure
typedef struct amd_pcnet_device {
    pci_device_t* pci_dev;
    uint32_t io_base;
    mac_address_t mac_addr;
    
    // Descriptor rings (simplified)
    void* rx_ring;
    void* tx_ring;
    uint8_t* rx_buffers[16];
    uint8_t* tx_buffers[16];
    uint16_t rx_cur;
    uint16_t tx_cur;
    
    int initialized;
} amd_pcnet_device_t;

// Function declarations
int amd_pcnet_init(void);
int amd_pcnet_detect_device(void);
int amd_pcnet_setup_device(pci_device_t* pci_dev);
void amd_pcnet_read_mac_address(amd_pcnet_device_t* dev);
int amd_pcnet_setup_rings(amd_pcnet_device_t* dev);
int amd_pcnet_send_packet(const void* data, uint32_t len);
int amd_pcnet_receive_packet(void* buffer, uint32_t max_len);
uint16_t amd_pcnet_read_csr(amd_pcnet_device_t* dev, uint16_t reg);
void amd_pcnet_write_csr(amd_pcnet_device_t* dev, uint16_t reg, uint16_t value);
amd_pcnet_device_t* get_amd_pcnet_device(void);

#endif