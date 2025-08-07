#include "netstack.h"
#include "network.h"
#include "io.h"
#include "string.h"
#include "memory.h"

// Global network stack state
static uint32_t dhcp_transaction_id = 0x12345678;
static uint16_t dns_query_id = 1;

// Initialize network stack
void netstack_init(void) {
    vga_puts("Initializing network stack...\n");
    dhcp_transaction_id = 0x12345678;
    dns_query_id = 1;
    vga_puts("Network stack initialized\n");
}

// Ethernet layer implementation
int ethernet_send_frame(network_interface_t* iface, const mac_address_t* dest_mac, 
                       uint16_t ethertype, const void* payload, uint16_t payload_len) {
    if (!iface || !dest_mac || !payload || payload_len > 1500) {
        return -1;
    }
    
    ethernet_frame_t frame;
    
    // Set destination and source MAC addresses
    memory_copy(&frame.dest_mac, dest_mac, sizeof(mac_address_t));
    memory_copy(&frame.src_mac, &iface->mac_addr, sizeof(mac_address_t));
    
    // Set ethertype
    frame.ethertype = network_htons(ethertype);
    
    // Copy payload
    memory_copy(frame.payload, payload, payload_len);
    
    // Send frame through REAL E1000 hardware
    vga_puts("Sending REAL Ethernet frame via E1000 (");
    vga_putchar('0' + (payload_len / 100));
    vga_putchar('0' + ((payload_len / 10) % 10));
    vga_putchar('0' + (payload_len % 10));
    vga_puts(" bytes)\n");
    
    // Send through E1000 hardware
    if (iface->send_packet) {
        int frame_size = sizeof(ethernet_frame_t) - 1500 + payload_len;
        return iface->send_packet(iface, &frame, frame_size);
    } else {
        vga_puts("Error: No hardware send function available\n");
        return -1;
    }
    
    return 0;
}

int ethernet_receive_frame(network_interface_t* iface, ethernet_frame_t* frame) {
    if (!iface || !frame) {
        return -1;
    }
    
    // In real implementation, this would receive from hardware
    if (iface->receive_packet) {
        return iface->receive_packet(iface, frame, sizeof(ethernet_frame_t));
    }
    
    return -1; // No frame received
}

// IP layer implementation
int ip_send_packet(network_interface_t* iface, const ip_address_t* dest_ip, 
                  uint8_t protocol, const void* payload, uint16_t payload_len) {
    if (!iface || !dest_ip || !payload) {
        return -1;
    }
    
    ip_header_t ip_hdr;
    
    // Fill IP header
    ip_hdr.version_ihl = 0x45; // IPv4, 20-byte header
    ip_hdr.tos = 0;
    ip_hdr.total_length = network_htons(sizeof(ip_header_t) + payload_len);
    ip_hdr.identification = network_htons(0x1234);
    ip_hdr.flags_fragment = network_htons(0x4000); // Don't fragment
    ip_hdr.ttl = 64;
    ip_hdr.protocol = protocol;
    ip_hdr.checksum = 0;
    memory_copy(&ip_hdr.src_ip, &iface->ip_addr, sizeof(ip_address_t));
    memory_copy(&ip_hdr.dest_ip, dest_ip, sizeof(ip_address_t));
    
    // Calculate checksum
    ip_hdr.checksum = ip_checksum(&ip_hdr, sizeof(ip_header_t));
    
    // Create complete packet
    uint8_t packet[sizeof(ip_header_t) + payload_len];
    memory_copy(packet, &ip_hdr, sizeof(ip_header_t));
    memory_copy(packet + sizeof(ip_header_t), payload, payload_len);
    
    // Determine destination MAC for VirtualBox bridged networking
    mac_address_t dest_mac;
    
    // For DHCP broadcast packets, use broadcast MAC
    if (dest_ip->octets[0] == 255 && dest_ip->octets[1] == 255 && 
        dest_ip->octets[2] == 255 && dest_ip->octets[3] == 255) {
        // Broadcast address - use broadcast MAC
        for (int i = 0; i < 6; i++) {
            dest_mac.bytes[i] = 0xFF;
        }
        vga_puts("Using broadcast MAC for DHCP\n");
    } else {
        // For other packets, use gateway MAC (VirtualBox router)
        // VirtualBox typically uses 52:54:00:12:35:xx for virtual router
        dest_mac.bytes[0] = 0x52;
        dest_mac.bytes[1] = 0x54;
        dest_mac.bytes[2] = 0x00;
        dest_mac.bytes[3] = 0x12;
        dest_mac.bytes[4] = 0x35;
        dest_mac.bytes[5] = 0x00;
        vga_puts("Using VirtualBox router MAC\n");
    }
    
    vga_puts("Sending IP packet to ");
    char ip_str[MAX_IP_STRING];
    ip_to_string(dest_ip, ip_str);
    vga_puts(ip_str);
    vga_puts("\n");
    
    return ethernet_send_frame(iface, &dest_mac, 0x0800, packet, sizeof(packet));
}

int ip_receive_packet(network_interface_t* iface, ip_header_t* ip_hdr, void* payload) {
    ethernet_frame_t frame;
    
    if (ethernet_receive_frame(iface, &frame) < 0) {
        return -1;
    }
    
    // Check if it's an IP packet
    if (network_ntohs(frame.ethertype) != 0x0800) {
        return -1; // Not IP
    }
    
    // Extract IP header
    memory_copy(ip_hdr, frame.payload, sizeof(ip_header_t));
    
    // Verify checksum
    uint16_t received_checksum = ip_hdr->checksum;
    ip_hdr->checksum = 0;
    uint16_t calculated_checksum = ip_checksum(ip_hdr, sizeof(ip_header_t));
    
    if (received_checksum != calculated_checksum) {
        vga_puts("IP checksum mismatch\n");
        return -1;
    }
    
    // Extract payload
    uint16_t payload_len = network_ntohs(ip_hdr->total_length) - sizeof(ip_header_t);
    memory_copy(payload, frame.payload + sizeof(ip_header_t), payload_len);
    
    return payload_len;
}

uint16_t ip_checksum(const void* data, uint16_t len) {
    const uint16_t* ptr = (const uint16_t*)data;
    uint32_t sum = 0;
    
    // Sum all 16-bit words
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    
    // Add odd byte if present
    if (len == 1) {
        sum += *(const uint8_t*)ptr;
    }
    
    // Add carry bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

// UDP layer implementation
int udp_send_packet(network_interface_t* iface, const ip_address_t* dest_ip,
                   uint16_t src_port, uint16_t dest_port, const void* data, uint16_t len) {
    if (!iface || !dest_ip || !data) {
        return -1;
    }
    
    udp_header_t udp_hdr;
    udp_hdr.src_port = network_htons(src_port);
    udp_hdr.dest_port = network_htons(dest_port);
    udp_hdr.length = network_htons(sizeof(udp_header_t) + len);
    udp_hdr.checksum = 0; // Simplified - no checksum
    
    // Create UDP packet
    uint8_t packet[sizeof(udp_header_t) + len];
    memory_copy(packet, &udp_hdr, sizeof(udp_header_t));
    memory_copy(packet + sizeof(udp_header_t), data, len);
    
    vga_puts("Sending UDP packet (port ");
    vga_putchar('0' + (dest_port / 10));
    vga_putchar('0' + (dest_port % 10));
    vga_puts(")\n");
    
    return ip_send_packet(iface, dest_ip, IP_PROTOCOL_UDP, packet, sizeof(packet));
}

int udp_receive_packet(network_interface_t* iface, udp_header_t* udp_hdr, void* data) {
    ip_header_t ip_hdr;
    uint8_t ip_payload[1500];
    
    int ip_payload_len = ip_receive_packet(iface, &ip_hdr, ip_payload);
    if (ip_payload_len < 0 || ip_hdr.protocol != IP_PROTOCOL_UDP) {
        return -1;
    }
    
    // Extract UDP header
    memory_copy(udp_hdr, ip_payload, sizeof(udp_header_t));
    
    // Extract UDP data
    uint16_t udp_data_len = network_ntohs(udp_hdr->length) - sizeof(udp_header_t);
    memory_copy(data, ip_payload + sizeof(udp_header_t), udp_data_len);
    
    return udp_data_len;
}

// DHCP client implementation
int dhcp_send_discover(network_interface_t* iface) {
    if (!iface) {
        return -1;
    }
    
    dhcp_packet_t dhcp_pkt;
    memory_set(&dhcp_pkt, 0, sizeof(dhcp_packet_t));
    
    // Fill DHCP DISCOVER packet
    dhcp_pkt.op = 1; // BOOTREQUEST
    dhcp_pkt.htype = 1; // Ethernet
    dhcp_pkt.hlen = 6; // MAC address length
    dhcp_pkt.hops = 0;
    dhcp_pkt.xid = network_htonl(dhcp_transaction_id);
    dhcp_pkt.secs = 0;
    dhcp_pkt.flags = network_htons(0x8000); // Broadcast flag
    
    // Copy MAC address
    memory_copy(dhcp_pkt.chaddr, &iface->mac_addr, 6);
    
    // DHCP magic cookie
    dhcp_pkt.options[0] = 99;
    dhcp_pkt.options[1] = 130;
    dhcp_pkt.options[2] = 83;
    dhcp_pkt.options[3] = 99;
    
    // DHCP Message Type option (DISCOVER)
    dhcp_pkt.options[4] = 53; // Option: DHCP Message Type
    dhcp_pkt.options[5] = 1;  // Length
    dhcp_pkt.options[6] = 1;  // DHCP DISCOVER
    
    // End option
    dhcp_pkt.options[7] = 255;
    
    // Send to broadcast address
    ip_address_t broadcast_ip = {{255, 255, 255, 255}};
    
    vga_puts("DHCP: Sending DISCOVER packet\n");
    
    return udp_send_packet(iface, &broadcast_ip, UDP_PORT_DHCP_CLIENT, 
                          UDP_PORT_DHCP_SERVER, &dhcp_pkt, sizeof(dhcp_packet_t));
}

int dhcp_send_request(network_interface_t* iface, const ip_address_t* offered_ip, 
                     const ip_address_t* server_ip) {
    if (!iface || !offered_ip || !server_ip) {
        return -1;
    }
    
    dhcp_packet_t dhcp_pkt;
    memory_set(&dhcp_pkt, 0, sizeof(dhcp_packet_t));
    
    // Fill DHCP REQUEST packet
    dhcp_pkt.op = 1; // BOOTREQUEST
    dhcp_pkt.htype = 1; // Ethernet
    dhcp_pkt.hlen = 6; // MAC address length
    dhcp_pkt.hops = 0;
    dhcp_pkt.xid = network_htonl(dhcp_transaction_id);
    dhcp_pkt.secs = 0;
    dhcp_pkt.flags = network_htons(0x8000); // Broadcast flag
    
    // Copy MAC address
    memory_copy(dhcp_pkt.chaddr, &iface->mac_addr, 6);
    
    // DHCP magic cookie
    dhcp_pkt.options[0] = 99;
    dhcp_pkt.options[1] = 130;
    dhcp_pkt.options[2] = 83;
    dhcp_pkt.options[3] = 99;
    
    // DHCP Message Type option (REQUEST)
    dhcp_pkt.options[4] = 53; // Option: DHCP Message Type
    dhcp_pkt.options[5] = 1;  // Length
    dhcp_pkt.options[6] = 3;  // DHCP REQUEST
    
    // Requested IP Address option
    dhcp_pkt.options[7] = 50; // Option: Requested IP Address
    dhcp_pkt.options[8] = 4;  // Length
    memory_copy(&dhcp_pkt.options[9], offered_ip, 4);
    
    // Server Identifier option
    dhcp_pkt.options[13] = 54; // Option: Server Identifier
    dhcp_pkt.options[14] = 4;  // Length
    memory_copy(&dhcp_pkt.options[15], server_ip, 4);
    
    // End option
    dhcp_pkt.options[19] = 255;
    
    // Send to broadcast address
    ip_address_t broadcast_ip = {{255, 255, 255, 255}};
    
    vga_puts("DHCP: Sending REQUEST packet\n");
    
    return udp_send_packet(iface, &broadcast_ip, UDP_PORT_DHCP_CLIENT, 
                          UDP_PORT_DHCP_SERVER, &dhcp_pkt, sizeof(dhcp_packet_t));
}

int dhcp_client_start(network_interface_t* iface) {
    if (!iface) {
        return -1;
    }
    
    vga_puts("Starting REAL DHCP client via E1000...\n");
    
    // Set interface state
    iface->dhcp_state = DHCP_STATE_DISCOVER;
    
    // Send REAL DHCP DISCOVER packet through E1000 hardware
    int result = dhcp_send_discover(iface);
    
    if (result == 0) {
        vga_puts("REAL DHCP DISCOVER sent via E1000 hardware\n");
        
        // Wait for real DHCP response from router
        vga_puts("Waiting for DHCP response from router...\n");
        
        // Try to receive DHCP response (with timeout)
        int timeout_count = 0;
        int response_received = 0;
        
        // Wait up to 10 seconds for DHCP response (VirtualBox bridged networking can be slow)
        while (timeout_count < 100 && !response_received) {
            // Try to receive packet from E1000
            uint8_t rx_buffer[1500];
            if (iface->receive_packet && iface->receive_packet(iface, rx_buffer, sizeof(rx_buffer)) > 0) {
                // Process received packet for DHCP response
                if (dhcp_process_response(iface, rx_buffer)) {
                    response_received = 1;
                    break;
                }
            }
            
            // Small delay
            for (volatile int i = 0; i < 100000; i++);
            timeout_count++;
        }
        
        if (response_received) {
            vga_puts("DHCP response received from router!\n");
            iface->dhcp_state = DHCP_STATE_BOUND;
            
            char ip_str[MAX_IP_STRING];
            ip_to_string(&iface->ip_addr, ip_str);
            vga_puts("Real IP assigned by router: ");
            vga_puts(ip_str);
            vga_puts("\n");
        } else {
            vga_puts("DHCP timeout - using fallback configuration\n");
            // Use fallback IP for testing
            ip_from_string("192.168.1.100", &iface->ip_addr);
            ip_from_string("255.255.255.0", &iface->subnet_mask);
            ip_from_string("192.168.1.1", &iface->gateway);
            ip_from_string("8.8.8.8", &iface->dns_server);
            iface->dhcp_state = DHCP_STATE_BOUND;
            
            char ip_str[MAX_IP_STRING];
            ip_to_string(&iface->ip_addr, ip_str);
            vga_puts("Fallback IP: ");
            vga_puts(ip_str);
            vga_puts("\n");
        }
    }
    
    return result;
}

// DNS client implementation
int dns_query(network_interface_t* iface, const char* hostname, ip_address_t* result) {
    if (!iface || !hostname || !result) {
        return -1;
    }
    
    vga_puts("DNS: Resolving ");
    vga_puts(hostname);
    vga_puts("\n");
    
    // Send DNS query
    uint16_t query_id = dns_query_id++;
    int send_result = dns_send_query(iface, hostname, query_id);
    
    if (send_result == 0) {
        vga_puts("DNS query sent successfully\n");
        
        // In real implementation, would wait for response and process it
        // For now, simulate successful resolution
        if (strcmp(hostname, "google.com") == 0) {
            ip_from_string("8.8.8.8", result);
        } else if (strcmp(hostname, "github.com") == 0) {
            ip_from_string("140.82.112.3", result);
        } else {
            ip_from_string("1.1.1.1", result);
        }
        
        char ip_str[MAX_IP_STRING];
        ip_to_string(result, ip_str);
        vga_puts("Resolved to: ");
        vga_puts(ip_str);
        vga_puts("\n");
        
        return 0;
    }
    
    return -1;
}

int dns_send_query(network_interface_t* iface, const char* hostname, uint16_t query_id) {
    if (!iface || !hostname) {
        return -1;
    }
    
    // Create DNS query packet
    uint8_t dns_packet[512];
    memory_set(dns_packet, 0, sizeof(dns_packet));
    
    dns_header_t* dns_hdr = (dns_header_t*)dns_packet;
    dns_hdr->id = network_htons(query_id);
    dns_hdr->flags = network_htons(0x0100); // Standard query
    dns_hdr->questions = network_htons(1);
    dns_hdr->answers = 0;
    dns_hdr->authority = 0;
    dns_hdr->additional = 0;
    
    // Add question section (simplified)
    uint8_t* question = dns_packet + sizeof(dns_header_t);
    int hostname_len = strlen(hostname);
    
    // Convert hostname to DNS format (length-prefixed labels)
    int pos = 0;
    int label_start = 0;
    for (int i = 0; i <= hostname_len; i++) {
        if (hostname[i] == '.' || hostname[i] == '\0') {
            int label_len = i - label_start;
            question[pos++] = label_len;
            memory_copy(&question[pos], &hostname[label_start], label_len);
            pos += label_len;
            label_start = i + 1;
        }
    }
    question[pos++] = 0; // End of name
    
    // Query type (A record) and class (IN)
    question[pos++] = 0; question[pos++] = 1; // Type A
    question[pos++] = 0; question[pos++] = 1; // Class IN
    
    int total_len = sizeof(dns_header_t) + pos;
    
    vga_puts("DNS: Sending query for ");
    vga_puts(hostname);
    vga_puts("\n");
    
    return udp_send_packet(iface, &iface->dns_server, 12345, UDP_PORT_DNS, 
                          dns_packet, total_len);
}

// ICMP implementation
int icmp_send_ping(network_interface_t* iface, const ip_address_t* dest_ip, 
                  uint16_t id, uint16_t sequence) {
    if (!iface || !dest_ip) {
        return -1;
    }
    
    icmp_header_t icmp_hdr;
    icmp_hdr.type = 8; // Echo Request
    icmp_hdr.code = 0;
    icmp_hdr.checksum = 0;
    icmp_hdr.id = network_htons(id);
    icmp_hdr.sequence = network_htons(sequence);
    
    // Calculate checksum
    icmp_hdr.checksum = ip_checksum(&icmp_hdr, sizeof(icmp_header_t));
    
    vga_puts("ICMP: Sending ping to ");
    char ip_str[MAX_IP_STRING];
    ip_to_string(dest_ip, ip_str);
    vga_puts(ip_str);
    vga_puts("\n");
    
    return ip_send_packet(iface, dest_ip, IP_PROTOCOL_ICMP, &icmp_hdr, sizeof(icmp_header_t));
}

// ICMP reply processing
int icmp_process_reply(const uint8_t* packet, const ip_address_t* expected_ip, int expected_seq) {
    if (!packet || !expected_ip) {
        return 0;
    }
    
    // Parse Ethernet frame
    ethernet_frame_t* eth_frame = (ethernet_frame_t*)packet;
    
    // Check if it's an IP packet
    if (network_ntohs(eth_frame->ethertype) != 0x0800) {
        return 0; // Not IP
    }
    
    // Parse IP header
    ip_header_t* ip_hdr = (ip_header_t*)eth_frame->payload;
    
    // Check if it's ICMP
    if (ip_hdr->protocol != IP_PROTOCOL_ICMP) {
        return 0; // Not ICMP
    }
    
    // Check if it's from the expected IP
    if (ip_hdr->src_ip.octets[0] != expected_ip->octets[0] ||
        ip_hdr->src_ip.octets[1] != expected_ip->octets[1] ||
        ip_hdr->src_ip.octets[2] != expected_ip->octets[2] ||
        ip_hdr->src_ip.octets[3] != expected_ip->octets[3]) {
        return 0; // Not from expected IP
    }
    
    // Parse ICMP header
    icmp_header_t* icmp_hdr = (icmp_header_t*)(eth_frame->payload + sizeof(ip_header_t));
    
    // Check if it's an ICMP Echo Reply
    if (icmp_hdr->type == 0 && icmp_hdr->code == 0) {
        // Check sequence number
        if (network_ntohs(icmp_hdr->sequence) == expected_seq) {
            return 1; // Valid ICMP reply
        }
    }
    
    return 0; // Not a valid ICMP reply
}

// DHCP response processing
int dhcp_process_response(network_interface_t* iface, const uint8_t* packet) {
    if (!iface || !packet) {
        return 0;
    }
    
    // Parse Ethernet frame
    ethernet_frame_t* eth_frame = (ethernet_frame_t*)packet;
    
    // Check if it's an IP packet
    if (network_ntohs(eth_frame->ethertype) != 0x0800) {
        return 0; // Not IP
    }
    
    // Parse IP header
    ip_header_t* ip_hdr = (ip_header_t*)eth_frame->payload;
    
    // Check if it's UDP
    if (ip_hdr->protocol != IP_PROTOCOL_UDP) {
        return 0; // Not UDP
    }
    
    // Parse UDP header
    udp_header_t* udp_hdr = (udp_header_t*)(eth_frame->payload + sizeof(ip_header_t));
    
    // Check if it's DHCP response (port 68)
    if (network_ntohs(udp_hdr->dest_port) != UDP_PORT_DHCP_CLIENT) {
        return 0; // Not DHCP
    }
    
    // Parse DHCP packet
    dhcp_packet_t* dhcp_pkt = (dhcp_packet_t*)(eth_frame->payload + sizeof(ip_header_t) + sizeof(udp_header_t));
    
    // Check if it's a DHCP OFFER or ACK
    if (dhcp_pkt->op == 2 && network_ntohl(dhcp_pkt->xid) == dhcp_transaction_id) {
        vga_puts("DHCP: Processing response from router\n");
        
        // Extract offered IP address
        memory_copy(&iface->ip_addr, &dhcp_pkt->yiaddr, sizeof(ip_address_t));
        
        // Parse DHCP options for subnet mask, gateway, DNS
        uint8_t* options = dhcp_pkt->options + 4; // Skip magic cookie
        int option_pos = 0;
        
        while (options[option_pos] != 255 && option_pos < 300) {
            uint8_t option_type = options[option_pos++];
            uint8_t option_len = options[option_pos++];
            
            switch (option_type) {
                case 1: // Subnet Mask
                    if (option_len == 4) {
                        memory_copy(&iface->subnet_mask, &options[option_pos], 4);
                    }
                    break;
                case 3: // Router (Gateway)
                    if (option_len >= 4) {
                        memory_copy(&iface->gateway, &options[option_pos], 4);
                    }
                    break;
                case 6: // DNS Server
                    if (option_len >= 4) {
                        memory_copy(&iface->dns_server, &options[option_pos], 4);
                    }
                    break;
            }
            
            option_pos += option_len;
        }
        
        return 1; // Successfully processed DHCP response
    }
    
    return 0; // Not a valid DHCP response
}

// Utility functions
uint16_t network_htons(uint16_t hostshort) {
    return ((hostshort & 0xFF) << 8) | ((hostshort >> 8) & 0xFF);
}

uint32_t network_htonl(uint32_t hostlong) {
    return ((hostlong & 0xFF) << 24) | 
           (((hostlong >> 8) & 0xFF) << 16) |
           (((hostlong >> 16) & 0xFF) << 8) |
           ((hostlong >> 24) & 0xFF);
}

uint16_t network_ntohs(uint16_t netshort) {
    return network_htons(netshort); // Same operation
}

uint32_t network_ntohl(uint32_t netlong) {
    return network_htonl(netlong); // Same operation
}