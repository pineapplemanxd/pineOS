#ifndef NETWORK_H
#define NETWORK_H

// Define our own integer types for bare-metal environment
typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;

// Network constants
#define MAX_NETWORK_INTERFACES 4
#define MAX_SSID_LENGTH 32
#define MAX_PASSWORD_LENGTH 64
#define MAX_WIFI_NETWORKS 16
#define MAX_IP_STRING 16
#define MAX_MAC_STRING 18

// Network interface types
#define NET_TYPE_ETHERNET 1
#define NET_TYPE_WIFI     2
#define NET_TYPE_LOOPBACK 3

// Network states
#define NET_STATE_DOWN        0
#define NET_STATE_UP          1
#define NET_STATE_CONNECTING  2
#define NET_STATE_CONNECTED   3
#define NET_STATE_ERROR       4

// DHCP states
#define DHCP_STATE_IDLE       0
#define DHCP_STATE_DISCOVER   1
#define DHCP_STATE_OFFER      2
#define DHCP_STATE_REQUEST    3
#define DHCP_STATE_BOUND      4

// WiFi security types
#define WIFI_SECURITY_NONE    0
#define WIFI_SECURITY_WEP     1
#define WIFI_SECURITY_WPA     2
#define WIFI_SECURITY_WPA2    3
#define WIFI_SECURITY_WPA3    4

// IP address structure
typedef struct ip_address {
    uint8_t octets[4];
} ip_address_t;

// MAC address structure
typedef struct mac_address {
    uint8_t bytes[6];
} mac_address_t;

// WiFi network information
typedef struct wifi_network {
    char ssid[MAX_SSID_LENGTH];
    uint8_t security_type;
    int signal_strength;
    mac_address_t bssid;
    uint8_t channel;
    int used;
} wifi_network_t;

// Network interface structure
typedef struct network_interface {
    char name[16];
    uint8_t type;
    uint8_t state;
    mac_address_t mac_addr;
    ip_address_t ip_addr;
    ip_address_t subnet_mask;
    ip_address_t gateway;
    ip_address_t dns_server;
    uint8_t dhcp_state;
    int used;
    
    // WiFi specific
    char connected_ssid[MAX_SSID_LENGTH];
    int signal_strength;
    
    // Interface functions
    int (*send_packet)(struct network_interface* iface, const void* data, uint32_t size);
    int (*receive_packet)(struct network_interface* iface, void* buffer, uint32_t max_size);
    int (*set_ip)(struct network_interface* iface, ip_address_t ip, ip_address_t mask);
} network_interface_t;

// Network initialization
void network_init(void);

// Interface management
network_interface_t* network_create_interface(const char* name, uint8_t type);
network_interface_t* network_get_interface(const char* name);
void network_list_interfaces(void);
int network_interface_up(const char* name);
int network_interface_down(const char* name);

// IP configuration
int network_set_static_ip(const char* interface, const char* ip, const char* mask, const char* gateway);
int network_start_dhcp(const char* interface);
void network_show_config(const char* interface);

// WiFi functions
int wifi_scan_networks(void);
void wifi_list_networks(void);
int wifi_connect(const char* ssid, const char* password);
int wifi_disconnect(void);
void wifi_show_status(void);

// Network utilities
int ping(const char* target, int count);
void network_show_stats(void);

// IP address utilities
int ip_from_string(const char* str, ip_address_t* ip);
void ip_to_string(const ip_address_t* ip, char* str);
void mac_to_string(const mac_address_t* mac, char* str);

// DHCP client
int dhcp_discover(network_interface_t* iface);
int dhcp_request(network_interface_t* iface, ip_address_t offered_ip);

// Network packet handling
int network_send_packet(const char* interface, const void* data, uint32_t size);
int network_receive_packet(const char* interface, void* buffer, uint32_t max_size);

// Forward declaration for PCI device
typedef struct pci_device pci_device_t;

// Hardware-specific functions
int wifi_hardware_init(void);
int wifi_hardware_scan(void);
int wifi_init_intel(pci_device_t* device);
int wifi_init_realtek(pci_device_t* device);
int wifi_init_broadcom(pci_device_t* device);
int wifi_init_atheros(pci_device_t* device);
int network_real_dhcp(const char* interface);
int network_dns_resolve(const char* hostname, ip_address_t* result);

// Additional WiFi functions
int wifi_start_scan(void);
int wifi_process_scan_results(void);

// Global WiFi state (for external access)
extern wifi_network_t* get_wifi_networks(void);
extern int* get_wifi_network_count(void);

// Real networking functions
int real_network_init(void);
int e1000_init(void);
int ax201_init(void);
int ax201_send_packet(const void* data, uint32_t len);
int ax201_receive_packet(void* buffer, uint32_t max_len);
int amd_pcnet_init(void);
int amd_pcnet_send_packet(const void* data, uint32_t len);
int amd_pcnet_receive_packet(void* buffer, uint32_t max_len);
int network_real_ping(const char* target, int count);

// Network stack integration
void netstack_init(void);
int dhcp_client_start(network_interface_t* iface);
int dns_query(network_interface_t* iface, const char* hostname, ip_address_t* result);
int icmp_send_ping(network_interface_t* iface, const ip_address_t* dest_ip, 
                  uint16_t id, uint16_t sequence);

#endif