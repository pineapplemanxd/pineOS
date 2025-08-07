#ifndef STORAGE_H
#define STORAGE_H

// Define our own integer types for bare-metal environment
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

// Storage device types
#define STORAGE_TYPE_UNKNOWN 0
#define STORAGE_TYPE_FLOPPY  1
#define STORAGE_TYPE_HDD     2
#define STORAGE_TYPE_USB     3

// Storage device structure
typedef struct storage_device {
    int type;
    uint32_t sector_size;
    uint32_t total_sectors;
    char name[32];
    int (*read_sector)(struct storage_device* dev, uint32_t sector, void* buffer);
    int (*write_sector)(struct storage_device* dev, uint32_t sector, const void* buffer);
} storage_device_t;

// Storage management
void storage_init(void);
int storage_detect_devices(void);
storage_device_t* storage_get_device(int index);
int storage_get_device_count(void);

// USB storage functions
int usb_storage_init(void);
int usb_storage_read_sector(storage_device_t* dev, uint32_t sector, void* buffer);
int usb_storage_write_sector(storage_device_t* dev, uint32_t sector, const void* buffer);

// ATA/IDE disk functions
int ata_init(void);
int ata_read_sector(storage_device_t* dev, uint32_t sector, void* buffer);
int ata_write_sector(storage_device_t* dev, uint32_t sector, const void* buffer);

// Sector I/O functions
int storage_read_sectors(storage_device_t* dev, uint32_t start_sector, uint32_t count, void* buffer);
int storage_write_sectors(storage_device_t* dev, uint32_t start_sector, uint32_t count, const void* buffer);

#endif