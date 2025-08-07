#include "pci.h"
#include "io.h"

#define MAX_PCI_DEVICES 64

static pci_device_t pci_devices[MAX_PCI_DEVICES];
static int pci_device_count = 0;

// Initialize PCI subsystem
void pci_init(void) {
    vga_puts("Initializing PCI subsystem...\n");
    
    // Clear device array
    for (int i = 0; i < MAX_PCI_DEVICES; i++) {
        pci_devices[i].used = 0;
    }
    pci_device_count = 0;
    
    // Scan for PCI devices
    int found = pci_scan_devices();
    
    vga_puts("PCI scan complete. Found ");
    vga_putchar('0' + (found / 10));
    vga_putchar('0' + (found % 10));
    vga_puts(" devices\n");
}

// Read 32-bit value from PCI configuration space
uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

// Read 16-bit value from PCI configuration space
uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset & 0xFC);
    return (uint16_t)((dword >> ((offset & 2) * 8)) & 0xFFFF);
}

// Read 8-bit value from PCI configuration space
uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset & 0xFC);
    return (uint8_t)((dword >> ((offset & 3) * 8)) & 0xFF);
}

// Write 32-bit value to PCI configuration space
void pci_config_write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

// Scan for PCI devices
int pci_scan_devices(void) {
    pci_device_count = 0;
    
    for (uint8_t bus = 0; bus < 8; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint16_t vendor_id = pci_config_read_word(bus, device, function, PCI_VENDOR_ID);
                
                if (vendor_id == 0xFFFF) {
                    // No device present
                    if (function == 0) break; // Skip other functions if function 0 doesn't exist
                    continue;
                }
                
                if (pci_device_count >= MAX_PCI_DEVICES) {
                    vga_puts("Warning: Too many PCI devices, some may not be detected\n");
                    return pci_device_count;
                }
                
                pci_device_t* dev = &pci_devices[pci_device_count];
                dev->bus = bus;
                dev->device = device;
                dev->function = function;
                dev->vendor_id = vendor_id;
                dev->device_id = pci_config_read_word(bus, device, function, PCI_DEVICE_ID);
                dev->class_code = pci_config_read_byte(bus, device, function, PCI_CLASS_CODE);
                dev->subclass = pci_config_read_byte(bus, device, function, PCI_SUBCLASS);
                dev->interrupt_line = pci_config_read_byte(bus, device, function, PCI_INTERRUPT_LINE);
                
                // Read BARs
                for (int i = 0; i < 6; i++) {
                    dev->bar[i] = pci_config_read_dword(bus, device, function, PCI_BAR0 + (i * 4));
                }
                
                dev->used = 1;
                pci_device_count++;
                
                // If this is not a multi-function device, skip other functions
                if (function == 0) {
                    uint8_t header_type = pci_config_read_byte(bus, device, function, 0x0E);
                    if ((header_type & 0x80) == 0) {
                        break; // Not multi-function
                    }
                }
            }
        }
    }
    
    return pci_device_count;
}

// Find PCI device by vendor and device ID
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].used && 
            pci_devices[i].vendor_id == vendor_id && 
            pci_devices[i].device_id == device_id) {
            return &pci_devices[i];
        }
    }
    return 0;
}

// Find PCI device by class code
pci_device_t* pci_find_class(uint8_t class_code, uint8_t subclass) {
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].used && 
            pci_devices[i].class_code == class_code && 
            pci_devices[i].subclass == subclass) {
            return &pci_devices[i];
        }
    }
    return 0;
}

// List all PCI devices
void pci_list_devices(void) {
    vga_puts("PCI Devices:\n");
    vga_puts("Bus Dev Fn Vendor Device Class Sub IRQ Description\n");
    vga_puts("--- --- -- ------ ------ ----- --- --- -----------\n");
    
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].used) {
            pci_device_t* dev = &pci_devices[i];
            
            // Bus
            vga_putchar('0' + (dev->bus / 10));
            vga_putchar('0' + (dev->bus % 10));
            vga_putchar(' ');
            
            // Device
            vga_putchar('0' + (dev->device / 10));
            vga_putchar('0' + (dev->device % 10));
            vga_putchar(' ');
            
            // Function
            vga_putchar('0' + dev->function);
            vga_putchar(' ');
            
            // Vendor ID (hex)
            const char hex[] = "0123456789ABCDEF";
            vga_putchar(hex[(dev->vendor_id >> 12) & 0xF]);
            vga_putchar(hex[(dev->vendor_id >> 8) & 0xF]);
            vga_putchar(hex[(dev->vendor_id >> 4) & 0xF]);
            vga_putchar(hex[dev->vendor_id & 0xF]);
            vga_putchar(' ');
            
            // Device ID (hex)
            vga_putchar(hex[(dev->device_id >> 12) & 0xF]);
            vga_putchar(hex[(dev->device_id >> 8) & 0xF]);
            vga_putchar(hex[(dev->device_id >> 4) & 0xF]);
            vga_putchar(hex[dev->device_id & 0xF]);
            vga_putchar(' ');
            
            // Class
            vga_putchar(hex[(dev->class_code >> 4) & 0xF]);
            vga_putchar(hex[dev->class_code & 0xF]);
            vga_putchar(' ');
            
            // Subclass
            vga_putchar(hex[(dev->subclass >> 4) & 0xF]);
            vga_putchar(hex[dev->subclass & 0xF]);
            vga_putchar(' ');
            
            // IRQ
            vga_putchar('0' + (dev->interrupt_line / 10));
            vga_putchar('0' + (dev->interrupt_line % 10));
            vga_putchar(' ');
            
            // Description
            if (dev->class_code == PCI_CLASS_NETWORK) {
                if (dev->subclass == PCI_SUBCLASS_ETHERNET) {
                    vga_puts("Ethernet Controller");
                } else if (dev->subclass == PCI_SUBCLASS_WIFI) {
                    vga_puts("WiFi Controller");
                } else {
                    vga_puts("Network Controller");
                }
            } else {
                vga_puts("Unknown Device");
            }
            
            vga_puts("\n");
        }
    }
}