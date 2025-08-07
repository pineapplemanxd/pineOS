#ifndef PCI_H
#define PCI_H

// Define our own integer types for bare-metal environment
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

// PCI Configuration Space Registers
#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

// PCI Header Type 0 offsets
#define PCI_VENDOR_ID       0x00
#define PCI_DEVICE_ID       0x02
#define PCI_COMMAND         0x04
#define PCI_STATUS          0x06
#define PCI_CLASS_CODE      0x0B
#define PCI_SUBCLASS        0x0A
#define PCI_PROG_IF         0x09
#define PCI_REVISION_ID     0x08
#define PCI_BAR0            0x10
#define PCI_BAR1            0x14
#define PCI_BAR2            0x18
#define PCI_BAR3            0x1C
#define PCI_BAR4            0x20
#define PCI_BAR5            0x24
#define PCI_INTERRUPT_LINE  0x3C
#define PCI_INTERRUPT_PIN   0x3D

// PCI Class Codes
#define PCI_CLASS_NETWORK   0x02
#define PCI_SUBCLASS_ETHERNET 0x00
#define PCI_SUBCLASS_WIFI   0x80

// Common Network Card Vendor/Device IDs
#define VENDOR_INTEL        0x8086
#define VENDOR_REALTEK      0x10EC
#define VENDOR_BROADCOM     0x14E4
#define VENDOR_ATHEROS      0x168C

// VirtualBox Network Adapter IDs
#define VBOX_VENDOR_ID      0x8086
#define VBOX_E1000_DEVICE   0x100E  // Intel E1000 (VirtualBox default)
#define VBOX_E1000E_DEVICE  0x10D3  // Intel E1000E
#define VBOX_VIRTIO_NET     0x1000  // VirtIO Network

// AMD Network Adapter IDs (common in some VirtualBox setups)
#define VENDOR_AMD          0x1022
#define AMD_PCNET_DEVICE    0x2000  // AMD PCnet (your device!)

// Intel WiFi Device IDs
#define INTEL_WIFI_AC7260   0x08B1
#define INTEL_WIFI_AC8260   0x24F3
#define INTEL_WIFI_AC9260   0x2526
#define INTEL_WIFI_AX200    0x2723

// Realtek Ethernet Device IDs
#define REALTEK_RTL8139     0x8139
#define REALTEK_RTL8169     0x8169

// PCI Device Structure
typedef struct pci_device {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint32_t bar[6];
    uint8_t interrupt_line;
    int used;
} pci_device_t;

// PCI Functions
void pci_init(void);
uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pci_config_write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);
int pci_scan_devices(void);
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id);
pci_device_t* pci_find_class(uint8_t class_code, uint8_t subclass);
void pci_list_devices(void);

#endif