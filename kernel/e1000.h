#ifndef E1000_H
#define E1000_H

#include "network.h"
#include "pci.h"

// Intel E1000 Register Offsets
#define E1000_CTRL      0x00000  // Device Control
#define E1000_STATUS    0x00008  // Device Status
#define E1000_EECD      0x00010  // EEPROM Control
#define E1000_EERD      0x00014  // EEPROM Read
#define E1000_CTRL_EXT  0x00018  // Extended Device Control
#define E1000_FLA       0x0001C  // Flash Access
#define E1000_MDIC      0x00020  // MDI Control

// Interrupt Registers
#define E1000_ICR       0x000C0  // Interrupt Cause Read
#define E1000_ITR       0x000C4  // Interrupt Throttling Rate
#define E1000_ICS       0x000C8  // Interrupt Cause Set
#define E1000_IMS       0x000D0  // Interrupt Mask Set
#define E1000_IMC       0x000D8  // Interrupt Mask Clear

// Receive Registers
#define E1000_RCTL      0x00100  // Receive Control
#define E1000_RDTR      0x02820  // Receive Delay Timer
#define E1000_RADV      0x0282C  // Receive Interrupt Absolute Delay Timer
#define E1000_RSRPD     0x02C00  // Receive Small Packet Detect
#define E1000_RDBAL     0x02800  // Receive Descriptor Base Address Low
#define E1000_RDBAH     0x02804  // Receive Descriptor Base Address High
#define E1000_RDLEN     0x02808  // Receive Descriptor Length
#define E1000_RDH       0x02810  // Receive Descriptor Head
#define E1000_RDT       0x02818  // Receive Descriptor Tail

// Transmit Registers
#define E1000_TCTL      0x00400  // Transmit Control
#define E1000_TIPG      0x00410  // Transmit Inter Packet Gap
#define E1000_TDBAL     0x03800  // Transmit Descriptor Base Address Low
#define E1000_TDBAH     0x03804  // Transmit Descriptor Base Address High
#define E1000_TDLEN     0x03808  // Transmit Descriptor Length
#define E1000_TDH       0x03810  // Transmit Descriptor Head
#define E1000_TDT       0x03818  // Transmit Descriptor Tail

// Control Register Bits
#define E1000_CTRL_FD       0x00000001  // Full Duplex
#define E1000_CTRL_LRST     0x00000008  // Link Reset
#define E1000_CTRL_ASDE     0x00000020  // Auto-speed Detection Enable
#define E1000_CTRL_SLU      0x00000040  // Set Link Up
#define E1000_CTRL_ILOS     0x00000080  // Invert Loss of Signal
#define E1000_CTRL_SPD_SEL  0x00000300  // Speed Select
#define E1000_CTRL_FRCSPD   0x00000800  // Force Speed
#define E1000_CTRL_FRCDPLX  0x00001000  // Force Duplex
#define E1000_CTRL_RST      0x04000000  // Device Reset
#define E1000_CTRL_VME      0x40000000  // VLAN Mode Enable
#define E1000_CTRL_PHY_RST  0x80000000  // PHY Reset

// Receive Control Register Bits
#define E1000_RCTL_EN       0x00000002  // Receive Enable
#define E1000_RCTL_SBP      0x00000004  // Store Bad Packets
#define E1000_RCTL_UPE      0x00000008  // Unicast Promiscuous Enabled
#define E1000_RCTL_MPE      0x00000010  // Multicast Promiscuous Enabled
#define E1000_RCTL_LPE      0x00000020  // Long Packet Reception Enable
#define E1000_RCTL_LBM      0x000000C0  // Loopback Mode
#define E1000_RCTL_BAM      0x00008000  // Broadcast Accept Mode
#define E1000_RCTL_SZ_2048  0x00000000  // Receive Buffer Size 2048
#define E1000_RCTL_SECRC    0x04000000  // Strip Ethernet CRC

// Transmit Control Register Bits
#define E1000_TCTL_EN       0x00000002  // Transmit Enable
#define E1000_TCTL_PSP      0x00000008  // Pad Short Packets
#define E1000_TCTL_CT       0x00000ff0  // Collision Threshold
#define E1000_TCTL_COLD     0x003ff000  // Collision Distance

// Descriptor Status Bits
#define E1000_RXD_STAT_DD   0x01        // Descriptor Done
#define E1000_RXD_STAT_EOP  0x02        // End of Packet
#define E1000_TXD_STAT_DD   0x00000001  // Descriptor Done
#define E1000_TXD_CMD_EOP   0x01000000  // End of Packet
#define E1000_TXD_CMD_RS    0x08000000  // Report Status

// E1000 Descriptor Structures
typedef struct e1000_rx_desc {
    uint64_t buffer_addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed)) e1000_rx_desc_t;

typedef struct e1000_tx_desc {
    uint64_t buffer_addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed)) e1000_tx_desc_t;

// E1000 Device Structure
typedef struct e1000_device {
    pci_device_t* pci_dev;
    uint32_t mmio_base;
    mac_address_t mac_addr;
    e1000_rx_desc_t* rx_descs;
    e1000_tx_desc_t* tx_descs;
    uint8_t* rx_buffers[256];
    uint8_t* tx_buffers[256];
    uint16_t rx_cur;
    uint16_t tx_cur;
    int initialized;
} e1000_device_t;

// E1000 Function Declarations
int e1000_init(void);
int e1000_detect_device(void);
int e1000_setup_device(pci_device_t* pci_dev);
void e1000_read_mac_address(e1000_device_t* dev);
int e1000_setup_rx(e1000_device_t* dev);
int e1000_setup_tx(e1000_device_t* dev);
int e1000_send_packet(const void* data, uint32_t len);
int e1000_receive_packet(void* buffer, uint32_t max_len);
uint32_t e1000_read_reg(e1000_device_t* dev, uint32_t reg);
void e1000_write_reg(e1000_device_t* dev, uint32_t reg, uint32_t value);
e1000_device_t* get_e1000_device(void);

#endif