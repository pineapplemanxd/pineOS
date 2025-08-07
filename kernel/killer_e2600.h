#ifndef KILLER_E2600_H
#define KILLER_E2600_H

#include "network.h"
#include "pci.h"

// Killer E2600 Gigabit Ethernet Controller Constants
#define KILLER_VENDOR_ID        0x1969  // Qualcomm Atheros (Killer brand)
#define KILLER_E2600_DEVICE_ID  0xE0B1  // Killer E2600 Gigabit Ethernet
#define KILLER_E2500_DEVICE_ID  0xE0A1  // Killer E2500 (similar)
#define KILLER_E3000_DEVICE_ID  0xE0C1  // Killer E3000 (newer)

// Killer E2600 Register Offsets (based on Atheros AR816x family)
#define KILLER_MASTER_CTRL      0x1400  // Master Control
#define KILLER_IRQ_STATUS       0x1600  // Interrupt Status
#define KILLER_IRQ_MASK         0x1604  // Interrupt Mask
#define KILLER_MAC_STA_ADDR     0x1488  // MAC Station Address
#define KILLER_RX_BASE_ADDR_HI  0x1540  // RX Descriptor Base High
#define KILLER_RX_BASE_ADDR_LO  0x1544  // RX Descriptor Base Low
#define KILLER_TX_BASE_ADDR_HI  0x1580  // TX Descriptor Base High
#define KILLER_TX_BASE_ADDR_LO  0x1584  // TX Descriptor Base Low
#define KILLER_RX_BUF_SIZE      0x1548  // RX Buffer Size
#define KILLER_TX_BUF_SIZE      0x1588  // TX Buffer Size
#define KILLER_DMA_CTRL         0x1590  // DMA Control
#define KILLER_MAC_CTRL         0x1480  // MAC Control
#define KILLER_MDIO_CTRL        0x1414  // MDIO Control
#define KILLER_SERDES_LOCK      0x1424  // SerDes Lock

// Control Register Bits
#define KILLER_MASTER_CTRL_SOFT_RST     0x00000001  // Software Reset
#define KILLER_MASTER_CTRL_MTIMER_EN    0x00000002  // Master Timer Enable
#define KILLER_MASTER_CTRL_ITIMER_EN    0x00000004  // Interrupt Timer Enable
#define KILLER_MASTER_CTRL_MANUAL_INT   0x00000008  // Manual Interrupt
#define KILLER_MASTER_CTRL_REV_NUM      0x00FF0000  // Revision Number
#define KILLER_MASTER_CTRL_DEV_ID       0xFF000000  // Device ID

// MAC Control Register Bits
#define KILLER_MAC_CTRL_TX_EN           0x00000001  // Transmit Enable
#define KILLER_MAC_CTRL_RX_EN           0x00000002  // Receive Enable
#define KILLER_MAC_CTRL_TX_FLOW_EN      0x00000004  // TX Flow Control Enable
#define KILLER_MAC_CTRL_RX_FLOW_EN      0x00000008  // RX Flow Control Enable
#define KILLER_MAC_CTRL_LOOPBACK        0x00000010  // Loopback Mode
#define KILLER_MAC_CTRL_DUPLX           0x00000020  // Full Duplex
#define KILLER_MAC_CTRL_ADD_CRC         0x00000040  // Add CRC
#define KILLER_MAC_CTRL_PAD             0x00000080  // Pad Short Frames
#define KILLER_MAC_CTRL_LENCHK          0x00000100  // Length Check
#define KILLER_MAC_CTRL_HUGE_EN         0x00000200  // Huge Frame Enable
#define KILLER_MAC_CTRL_PRMLEN_SHIFT    10          // Preamble Length Shift
#define KILLER_MAC_CTRL_VLANSTRIP       0x00004000  // VLAN Strip
#define KILLER_MAC_CTRL_PROMISC         0x00008000  // Promiscuous Mode
#define KILLER_MAC_CTRL_MC_ALL          0x00010000  // All Multicast
#define KILLER_MAC_CTRL_BC_EN           0x00020000  // Broadcast Enable
#define KILLER_MAC_CTRL_SPEED_SHIFT     20          // Speed Selection Shift
#define KILLER_MAC_CTRL_SPEED_MASK      0x00300000  // Speed Selection Mask
#define KILLER_MAC_CTRL_SPEED_10_100    0x00100000  // 10/100 Mbps
#define KILLER_MAC_CTRL_SPEED_1000      0x00200000  // 1000 Mbps

// DMA Control Register Bits
#define KILLER_DMA_CTRL_DMAR_EN         0x00000001  // DMA Read Enable
#define KILLER_DMA_CTRL_DMAW_EN         0x00000002  // DMA Write Enable
#define KILLER_DMA_CTRL_DMAR_OUT_ORDER  0x00000004  // DMA Read Out of Order
#define KILLER_DMA_CTRL_DMAR_ENH_ORDER  0x00000008  // DMA Read Enhanced Order
#define KILLER_DMA_CTRL_DMAR_BURST_LEN  0x00000070  // DMA Read Burst Length
#define KILLER_DMA_CTRL_DMAW_BURST_LEN  0x00000700  // DMA Write Burst Length
#define KILLER_DMA_CTRL_DMAR_REQ_PRI    0x00000800  // DMA Read Request Priority
#define KILLER_DMA_CTRL_DMAR_DLY_CNT    0x0000F000  // DMA Read Delay Count
#define KILLER_DMA_CTRL_DMAW_DLY_CNT    0x000F0000  // DMA Write Delay Count

// Interrupt Status/Mask Bits
#define KILLER_IRQ_TX_PKT               0x00000001  // TX Packet Complete
#define KILLER_IRQ_RX_PKT               0x00000002  // RX Packet Available
#define KILLER_IRQ_TX_DMA               0x00000004  // TX DMA Complete
#define KILLER_IRQ_RX_DMA               0x00000008  // RX DMA Complete
#define KILLER_IRQ_SMB                  0x00000010  // SMBus Interrupt
#define KILLER_IRQ_PHY                  0x00000020  // PHY Interrupt
#define KILLER_IRQ_TX_CREDIT            0x00000040  // TX Credit Available
#define KILLER_IRQ_DMAW                 0x00000080  // DMA Write Complete
#define KILLER_IRQ_DMAR                 0x00000100  // DMA Read Complete
#define KILLER_IRQ_TX_EMPTY             0x00000200  // TX Queue Empty
#define KILLER_IRQ_RX_EMPTY             0x00000400  // RX Queue Empty

// Killer E2600 Descriptor Structures
typedef struct killer_rx_desc {
    uint64_t buffer_addr;
    uint16_t length;
    uint16_t flags;
    uint16_t vtag;
    uint16_t status;
} __attribute__((packed)) killer_rx_desc_t;

typedef struct killer_tx_desc {
    uint64_t buffer_addr;
    uint16_t length;
    uint16_t flags;
    uint16_t vtag;
    uint16_t status;
} __attribute__((packed)) killer_tx_desc_t;

// Killer E2600 Device Structure
typedef struct killer_e2600_device {
    pci_device_t* pci_dev;
    uint32_t mmio_base;
    uint32_t io_base;
    mac_address_t mac_addr;
    
    // Descriptor rings
    killer_rx_desc_t* rx_descs;
    killer_tx_desc_t* tx_descs;
    uint8_t* rx_buffers[256];
    uint8_t* tx_buffers[256];
    uint16_t rx_cur;
    uint16_t tx_cur;
    
    // Killer-specific features
    int gaming_accelerator_enabled;
    int advanced_stream_detect;
    int doubleshot_pro;
    uint32_t link_speed;  // 10, 100, 1000 Mbps
    int full_duplex;
    int initialized;
} killer_e2600_device_t;

// Function declarations
int killer_e2600_init(void);
int killer_e2600_detect_device(void);
int killer_e2600_setup_device(pci_device_t* pci_dev);
void killer_e2600_read_mac_address(killer_e2600_device_t* dev);
int killer_e2600_setup_rx(killer_e2600_device_t* dev);
int killer_e2600_setup_tx(killer_e2600_device_t* dev);
int killer_e2600_send_packet(const void* data, uint32_t len);
int killer_e2600_receive_packet(void* buffer, uint32_t max_len);
uint32_t killer_e2600_read_reg(killer_e2600_device_t* dev, uint32_t reg);
void killer_e2600_write_reg(killer_e2600_device_t* dev, uint32_t reg, uint32_t value);
killer_e2600_device_t* get_killer_e2600_device(void);

// Killer-specific features
int killer_e2600_enable_gaming_accelerator(void);
int killer_e2600_configure_doubleshot_pro(void);
int killer_e2600_setup_advanced_stream_detect(void);
void killer_e2600_show_performance_stats(void);

#endif