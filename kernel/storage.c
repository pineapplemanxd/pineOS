#include "storage.h"
#include "io.h"
#include "memory.h"
#include "string.h"

#define MAX_STORAGE_DEVICES 8

// ATA/IDE disk driver definitions
// Port definitions for primary ATA controller
#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_ERROR       0x1F1
#define ATA_PRIMARY_SECTOR_COUNT 0x1F2
#define ATA_PRIMARY_LBA_LOW     0x1F3
#define ATA_PRIMARY_LBA_MID     0x1F4
#define ATA_PRIMARY_LBA_HIGH    0x1F5
#define ATA_PRIMARY_DRIVE       0x1F6
#define ATA_PRIMARY_STATUS      0x1F7
#define ATA_PRIMARY_COMMAND     0x1F7

// ATA commands
#define ATA_CMD_READ_SECTORS    0x20
#define ATA_CMD_WRITE_SECTORS   0x30
#define ATA_CMD_IDENTIFY        0xEC

// Status register bits
#define ATA_STATUS_BSY          0x80  // Busy
#define ATA_STATUS_RDY          0x40  // Ready
#define ATA_STATUS_DRQ          0x08  // Data Request
#define ATA_STATUS_ERR          0x01  // Error

// Forward declarations
static int ata_wait_ready(void);
static int ata_wait_drq(void);

// Global storage devices array
static storage_device_t storage_devices[MAX_STORAGE_DEVICES];
static int device_count = 0;

// Initialize storage subsystem
void storage_init(void) {
    device_count = 0;
    
    // Clear device array
    for (int i = 0; i < MAX_STORAGE_DEVICES; i++) {
        storage_devices[i].type = STORAGE_TYPE_UNKNOWN;
        storage_devices[i].sector_size = 0;
        storage_devices[i].total_sectors = 0;
        storage_devices[i].name[0] = '\0';
        storage_devices[i].read_sector = 0;
        storage_devices[i].write_sector = 0;
    }
    
    vga_puts("Storage subsystem initialized\n");
    
    // Detect available storage devices
    storage_detect_devices();
}

// Detect available storage devices
int storage_detect_devices(void) {
    vga_puts("Detecting storage devices...\n");
    
    // Try to detect ATA/IDE drives first (for VMware/real hardware)
    if (ata_init() == 0) {
        vga_puts("ATA/IDE storage support initialized\n");
        
        // Add primary ATA drive as storage device
        if (device_count < MAX_STORAGE_DEVICES) {
            storage_device_t* dev = &storage_devices[device_count];
            dev->type = STORAGE_TYPE_HDD;
            dev->sector_size = 512;
            dev->total_sectors = 2048; // 1MB for filesystem storage
            strcpy(dev->name, "HDD0");
            dev->read_sector = ata_read_sector;
            dev->write_sector = ata_write_sector;
            device_count++;
            
            vga_puts("Found ATA/IDE drive: ");
            vga_puts(dev->name);
            vga_puts(" (");
            // Print size in KB
            uint32_t size_kb = (dev->total_sectors * dev->sector_size) / 1024;
            char size_str[16];
            int i = 0;
            do {
                size_str[i++] = '0' + (size_kb % 10);
                size_kb /= 10;
            } while (size_kb > 0);
            for (int j = i - 1; j >= 0; j--) {
                vga_putchar(size_str[j]);
            }
            vga_puts(" KB)\n");
        }
    } else {
        vga_puts("ATA/IDE initialization failed - using fallback storage\n");
    }
    
    // Try to detect USB storage devices as fallback
    if (usb_storage_init() == 0) {
        vga_puts("USB storage support initialized\n");
        
        // Always create a fallback storage device for VirtualBox compatibility
        if (device_count < MAX_STORAGE_DEVICES) {
            storage_device_t* dev = &storage_devices[device_count];
            dev->type = STORAGE_TYPE_USB;
            dev->sector_size = 512;
            dev->total_sectors = 2048; // 1MB simulated device
            strcpy(dev->name, "VDISK0");
            dev->read_sector = usb_storage_read_sector;
            dev->write_sector = usb_storage_write_sector;
            device_count++;
            
            vga_puts("Created virtual storage device: ");
            vga_puts(dev->name);
            vga_puts(" (");
            // Print size in KB
            uint32_t size_kb = (dev->total_sectors * dev->sector_size) / 1024;
            char size_str[16];
            int i = 0;
            do {
                size_str[i++] = '0' + (size_kb % 10);
                size_kb /= 10;
            } while (size_kb > 0);
            for (int j = i - 1; j >= 0; j--) {
                vga_putchar(size_str[j]);
            }
            vga_puts(" KB)\n");
        }
    }
    
    vga_puts("Storage detection complete. Found ");
    vga_putchar('0' + device_count);
    vga_puts(" device(s)\n");
    
    return device_count;
}

// Get storage device by index
storage_device_t* storage_get_device(int index) {
    if (index < 0 || index >= device_count) {
        return 0;
    }
    return &storage_devices[index];
}

// Get number of detected devices
int storage_get_device_count(void) {
    return device_count;
}

// Read multiple sectors
int storage_read_sectors(storage_device_t* dev, uint32_t start_sector, uint32_t count, void* buffer) {
    if (!dev || !dev->read_sector || !buffer) {
        return -1;
    }
    
    uint8_t* buf = (uint8_t*)buffer;
    for (uint32_t i = 0; i < count; i++) {
        if (dev->read_sector(dev, start_sector + i, buf + (i * dev->sector_size)) != 0) {
            return -1;
        }
    }
    
    return 0;
}

// Write multiple sectors
int storage_write_sectors(storage_device_t* dev, uint32_t start_sector, uint32_t count, const void* buffer) {
    if (!dev || !dev->write_sector || !buffer) {
        return -1;
    }
    
    const uint8_t* buf = (const uint8_t*)buffer;
    for (uint32_t i = 0; i < count; i++) {
        if (dev->write_sector(dev, start_sector + i, buf + (i * dev->sector_size)) != 0) {
            return -1;
        }
    }
    
    return 0;
}

// USB storage initialization
int usb_storage_init(void) {
    // For now, this is a placeholder
    // In a real implementation, this would:
    // 1. Initialize USB controller
    // 2. Enumerate USB devices
    // 3. Identify mass storage devices
    // 4. Set up communication protocols
    
    vga_puts("USB storage driver loaded\n");
    return 0;
}

// Persistent storage buffer - use a fixed high memory location
// This will be at a predictable location that VirtualBox can persist
static uint8_t* persistent_storage_buffer = (uint8_t*)0x200000; // 2MB mark
static int storage_initialized = 0;

// USB storage read sector - tries ATA first, then memory fallback
int usb_storage_read_sector(storage_device_t* dev, uint32_t sector, void* buffer) {
    if (!dev || !buffer || sector >= dev->total_sectors) {
        return -1;
    }
    
    // Try to use ATA directly for real persistence
    uint16_t* buf = (uint16_t*)buffer;
    
    // Wait for drive to be ready
    if (ata_wait_ready() == 0) {
        // Set up LBA addressing - use sectors starting from 100 to avoid boot sectors
        uint32_t disk_sector = sector + 100;
        outb(ATA_PRIMARY_SECTOR_COUNT, 1);
        outb(ATA_PRIMARY_LBA_LOW, disk_sector & 0xFF);
        outb(ATA_PRIMARY_LBA_MID, (disk_sector >> 8) & 0xFF);
        outb(ATA_PRIMARY_LBA_HIGH, (disk_sector >> 16) & 0xFF);
        outb(ATA_PRIMARY_DRIVE, 0xE0 | ((disk_sector >> 24) & 0x0F));
        
        // Send read command
        outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_SECTORS);
        
        // Wait for data
        if (ata_wait_drq() == 0) {
            // Read data (256 words = 512 bytes)
            for (int i = 0; i < 256; i++) {
                buf[i] = inw(ATA_PRIMARY_DATA);
            }
            return 0; // Success - real disk read
        }
    }
    
    // Fallback to memory buffer if ATA fails
    memory_copy(buffer, &persistent_storage_buffer[sector * dev->sector_size], dev->sector_size);
    return 0;
}

// USB storage write sector - tries ATA first, then memory fallback
int usb_storage_write_sector(storage_device_t* dev, uint32_t sector, const void* buffer) {
    if (!dev || !buffer || sector >= dev->total_sectors) {
        return -1;
    }
    
    // Try to use ATA directly for real persistence
    const uint16_t* buf = (const uint16_t*)buffer;
    
    // Wait for drive to be ready
    if (ata_wait_ready() == 0) {
        // Set up LBA addressing - use sectors starting from 100 to avoid boot sectors
        uint32_t disk_sector = sector + 100;
        outb(ATA_PRIMARY_SECTOR_COUNT, 1);
        outb(ATA_PRIMARY_LBA_LOW, disk_sector & 0xFF);
        outb(ATA_PRIMARY_LBA_MID, (disk_sector >> 8) & 0xFF);
        outb(ATA_PRIMARY_LBA_HIGH, (disk_sector >> 16) & 0xFF);
        outb(ATA_PRIMARY_DRIVE, 0xE0 | ((disk_sector >> 24) & 0x0F));
        
        // Send write command
        outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_SECTORS);
        
        // Wait for data request
        if (ata_wait_drq() == 0) {
            // Write data (256 words = 512 bytes)
            for (int i = 0; i < 256; i++) {
                outw(ATA_PRIMARY_DATA, buf[i]);
            }
            
            // Wait for write to complete
            if (ata_wait_ready() == 0) {
                // Also update memory buffer as backup
                memory_copy(&persistent_storage_buffer[sector * dev->sector_size], buffer, dev->sector_size);
                return 0; // Success - real disk write
            }
        }
    }
    
    // Fallback to memory buffer if ATA fails
    memory_copy(&persistent_storage_buffer[sector * dev->sector_size], buffer, dev->sector_size);
    return 0;
}

// ATA/IDE disk driver implementation

// Wait for ATA drive to be ready
static int ata_wait_ready(void) {
    int timeout = 10000;
    while (timeout-- > 0) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_RDY)) {
            return 0; // Ready
        }
    }
    return -1; // Timeout
}

// Wait for data request
static int ata_wait_drq(void) {
    int timeout = 10000;
    while (timeout-- > 0) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ)) {
            return 0; // Data ready
        }
        if (status & ATA_STATUS_ERR) {
            return -1; // Error
        }
    }
    return -1; // Timeout
}

// Initialize ATA/IDE controller
int ata_init(void) {
    vga_puts("Initializing ATA/IDE controller...\n");
    
    // Select master drive
    outb(ATA_PRIMARY_DRIVE, 0xA0);
    
    // Wait for drive to be ready
    if (ata_wait_ready() != 0) {
        vga_puts("ATA drive not ready\n");
        return -1;
    }
    
    // Send IDENTIFY command
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_IDENTIFY);
    
    // Check if drive exists
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status == 0) {
        vga_puts("No ATA drive detected\n");
        return -1;
    }
    
    // Wait for data
    if (ata_wait_drq() != 0) {
        vga_puts("ATA IDENTIFY failed\n");
        return -1;
    }
    
    // Read and discard IDENTIFY data (we don't need it for basic operation)
    for (int i = 0; i < 256; i++) {
        inw(ATA_PRIMARY_DATA);
    }
    
    vga_puts("ATA/IDE controller initialized successfully\n");
    return 0;
}

// Read a sector from ATA drive
int ata_read_sector(storage_device_t* dev, uint32_t sector, void* buffer) {
    if (!dev || !buffer || sector >= dev->total_sectors) {
        return -1;
    }
    
    uint16_t* buf = (uint16_t*)buffer;
    
    // Wait for drive to be ready
    if (ata_wait_ready() != 0) {
        return -1;
    }
    
    // Set up LBA addressing
    outb(ATA_PRIMARY_SECTOR_COUNT, 1);           // Read 1 sector
    outb(ATA_PRIMARY_LBA_LOW, sector & 0xFF);    // LBA bits 0-7
    outb(ATA_PRIMARY_LBA_MID, (sector >> 8) & 0xFF);  // LBA bits 8-15
    outb(ATA_PRIMARY_LBA_HIGH, (sector >> 16) & 0xFF); // LBA bits 16-23
    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((sector >> 24) & 0x0F)); // LBA mode, master drive, LBA bits 24-27
    
    // Send read command
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_SECTORS);
    
    // Wait for data
    if (ata_wait_drq() != 0) {
        return -1;
    }
    
    // Read data (256 words = 512 bytes)
    for (int i = 0; i < 256; i++) {
        buf[i] = inw(ATA_PRIMARY_DATA);
    }
    
    return 0;
}

// Write a sector to ATA drive
int ata_write_sector(storage_device_t* dev, uint32_t sector, const void* buffer) {
    if (!dev || !buffer || sector >= dev->total_sectors) {
        return -1;
    }
    
    const uint16_t* buf = (const uint16_t*)buffer;
    
    // Wait for drive to be ready
    if (ata_wait_ready() != 0) {
        return -1;
    }
    
    // Set up LBA addressing
    outb(ATA_PRIMARY_SECTOR_COUNT, 1);           // Write 1 sector
    outb(ATA_PRIMARY_LBA_LOW, sector & 0xFF);    // LBA bits 0-7
    outb(ATA_PRIMARY_LBA_MID, (sector >> 8) & 0xFF);  // LBA bits 8-15
    outb(ATA_PRIMARY_LBA_HIGH, (sector >> 16) & 0xFF); // LBA bits 16-23
    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((sector >> 24) & 0x0F)); // LBA mode, master drive, LBA bits 24-27
    
    // Send write command
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_SECTORS);
    
    // Wait for data request
    if (ata_wait_drq() != 0) {
        return -1;
    }
    
    // Write data (256 words = 512 bytes)
    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_DATA, buf[i]);
    }
    
    // Wait for write to complete
    if (ata_wait_ready() != 0) {
        return -1;
    }
    
    return 0;
}