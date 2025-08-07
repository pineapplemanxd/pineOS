#ifndef NETSTACK_H
#define NETSTACK_H

#include "network.h"

// Protocol numbers
#define IP_PROTOCOL_ICMP    1
#define IP_PROTOCOL_TCP     6
#define IP_PROTOCOL_UDP     17

// Port numbers
#define UDP_PORT_DHCP_CLIENT 68
#define UDP_PORT_DHCP_SERVER 67
#define UDP_PORT_DNS         53

// Ethernet frame structure
typedef struct ethernet_frame {
    mac_address_t dest_mac;
    mac_address_t src_mac;
    uint16_t ethertype;
    uint8_t payload[1500];
} __attribute__((packed)) ethernet_frame_t;

// IP header structure
typedef struct ip_header {
    uint8_t version_ihl;
    uint8_t tos;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_fragment;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    ip_address_t src_ip;
    ip_address_t dest_ip;
} __attribute__((packed)) ip_header_t;

// UDP header structure
typedef struct udp_header {
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed)) udp_header_t;

// DHCP packet structure
typedef struct dhcp_packet {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    ip_address_t ciaddr;
    ip_address_t yiaddr;
    ip_address_t siaddr;
    ip_address_t giaddr;
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint8_t options[312];
} __attribute__((packed)) dhcp_packet_t;

// DNS header structure
typedef struct dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t questions;
    uint16_t answers;
    uint16_t authority;
    uint16_t additional;
} __attribute__((packed)) dns_header_t;

// ICMP header structure
typedef struct icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
} __attribute__((packed)) icmp_header_t;

// Network stack functions
void netstack_init(void);

// Ethernet layer
int ethernet_send_frame(network_interface_t* iface, const mac_address_t* dest_mac, 
                       uint16_t ethertype, const void* payload, uint16_t payload_len);
int ethernet_receive_frame(network_interface_t* iface, ethernet_frame_t* frame);

// IP layer
int ip_send_packet(network_interface_t* iface, const ip_address_t* dest_ip, 
                  uint8_t protocol, const void* payload, uint16_t payload_len);
int ip_receive_packet(network_interface_t* iface, ip_header_t* ip_hdr, void* payload);
uint16_t ip_checksum(const void* data, uint16_t len);

// UDP layer
int udp_send_packet(network_interface_t* iface, const ip_address_t* dest_ip,
                   uint16_t src_port, uint16_t dest_port, const void* data, uint16_t len);
int udp_receive_packet(network_interface_t* iface, udp_header_t* udp_hdr, void* data);

// DHCP client implementation
int dhcp_send_discover(network_interface_t* iface);
int dhcp_send_request(network_interface_t* iface, const ip_address_t* offered_ip, 
                     const ip_address_t* server_ip);
int dhcp_process_offer(network_interface_t* iface, const dhcp_packet_t* packet);
int dhcp_process_ack(network_interface_t* iface, const dhcp_packet_t* packet);
int dhcp_client_start(network_interface_t* iface);

// DNS client implementation
int dns_query(network_interface_t* iface, const char* hostname, ip_address_t* result);
int dns_send_query(network_interface_t* iface, const char* hostname, uint16_t query_id);
int dns_process_response(const dns_header_t* dns_hdr, const void* data, 
                        uint16_t data_len, ip_address_t* result);

// ICMP implementation
int icmp_send_ping(network_interface_t* iface, const ip_address_t* dest_ip, 
                  uint16_t id, uint16_t sequence);
int icmp_process_reply(const uint8_t* packet, const ip_address_t* expected_ip, int expected_seq);

// DHCP response processing
int dhcp_process_response(network_interface_t* iface, const uint8_t* packet);

// Utility functions
uint16_t network_htons(uint16_t hostshort);
uint32_t network_htonl(uint32_t hostlong);
uint16_t network_ntohs(uint16_t netshort);
uint32_t network_ntohl(uint32_t netlong);

#endif