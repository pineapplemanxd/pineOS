#ifndef VIRTIO_NET_H
#define VIRTIO_NET_H

#include "network.h"

// VirtIO Network Device Constants
#define VIRTIO_NET_DEVICE_ID    0x1000
#define VIRTIO_VENDOR_ID        0x1AF4

// VirtIO PCI Configuration Registers
#define VIRTIO_PCI_HOST_FEATURES    0x00
#define VIRTIO_PCI_GUEST_FEATURES   0x04
#define VIRTIO_PCI_QUEUE_PFN        0x08
#define VIRTIO_PCI_QUEUE_SIZE       0x0C
#define VIRTIO_PCI_QUEUE_SELECT     0x0E
#define VIRTIO_PCI_QUEUE_NOTIFY     0x10
#define VIRTIO_PCI_STATUS           0x12
#define VIRTIO_PCI_ISR              0x13

// VirtIO Status Register Values
#define VIRTIO_STATUS_ACKNOWLEDGE   1
#define VIRTIO_STATUS_DRIVER        2
#define VIRTIO_STATUS_DRIVER_OK     4
#define VIRTIO_STATUS_FEATURES_OK   8
#define VIRTIO_STATUS_FAILED        128

// VirtIO Network Features
#define VIRTIO_NET_F_CSUM           0
#define VIRTIO_NET_F_GUEST_CSUM     1
#define VIRTIO_NET_F_MAC            5
#define VIRTIO_NET_F_GSO            6
#define VIRTIO_NET_F_GUEST_TSO4     7
#define VIRTIO_NET_F_GUEST_TSO6     8
#define VIRTIO_NET_F_GUEST_ECN      9
#define VIRTIO_NET_F_GUEST_UFO      10
#define VIRTIO_NET_F_HOST_TSO4      11
#define VIRTIO_NET_F_HOST_TSO6      12
#define VIRTIO_NET_F_HOST_ECN       13
#define VIRTIO_NET_F_HOST_UFO       14
#define VIRTIO_NET_F_MRG_RXBUF      15
#define VIRTIO_NET_F_STATUS         16
#define VIRTIO_NET_F_CTRL_VQ        17
#define VIRTIO_NET_F_CTRL_RX        18
#define VIRTIO_NET_F_CTRL_VLAN      19

// VirtIO Network Header
typedef struct virtio_net_hdr {
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
} __attribute__((packed)) virtio_net_hdr_t;

// VirtIO Queue Descriptor
typedef struct virtio_queue_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed)) virtio_queue_desc_t;

// VirtIO Available Ring
typedef struct virtio_queue_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[];
} __attribute__((packed)) virtio_queue_avail_t;

// VirtIO Used Ring Element
typedef struct virtio_queue_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed)) virtio_queue_used_elem_t;

// VirtIO Used Ring
typedef struct virtio_queue_used {
    uint16_t flags;
    uint16_t idx;
    virtio_queue_used_elem_t ring[];
} __attribute__((packed)) virtio_queue_used_t;

// VirtIO Queue
typedef struct virtio_queue {
    uint16_t size;
    virtio_queue_desc_t* desc;
    virtio_queue_avail_t* avail;
    virtio_queue_used_t* used;
    uint16_t last_used_idx;
    uint16_t free_head;
    uint16_t num_free;
} virtio_queue_t;

// VirtIO Network Device
typedef struct virtio_net_device {
    pci_device_t* pci_dev;
    uint32_t base_addr;
    mac_address_t mac_addr;
    virtio_queue_t rx_queue;
    virtio_queue_t tx_queue;
    uint8_t* rx_buffer;
    uint8_t* tx_buffer;
    int initialized;
} virtio_net_device_t;

// VirtIO Network Functions
int virtio_net_init(void);
int virtio_net_detect_device(void);
int virtio_net_setup_device(pci_device_t* pci_dev);
int virtio_net_setup_queues(virtio_net_device_t* dev);
int virtio_net_send_packet(const void* data, uint32_t len);
int virtio_net_receive_packet(void* buffer, uint32_t max_len);
void virtio_net_interrupt_handler(void);

// Real network interface functions
int real_network_init(void);
int real_wifi_scan(void);
int real_dhcp_request(network_interface_t* iface);
int real_dns_query(const char* hostname, ip_address_t* result);
int real_ping_send(const ip_address_t* target);

#endif