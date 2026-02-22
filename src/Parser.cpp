#include "Parser.h"
#include "Stats.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <iostream>
#include <iomanip>
#include <cstring>

// Ethernet header structure (14 bytes)
struct EthernetHdr {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t type;
} __attribute__((packed));

// IPv4 header structure
struct IPv4Hdr {
    uint8_t version_length;
    uint8_t tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t flags_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    struct in_addr src_ip;
    struct in_addr dst_ip;
} __attribute__((packed));

// TCP header structure
struct TCPHdr {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq;
    uint32_t ack;
    uint8_t offset_reserved;
    uint8_t flags;
    uint16_t window;
} __attribute__((packed));

// UDP header structure
struct UDPHdr {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed));

std::string format_mac(const uint8_t* mac) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(buf);
}

IPv4Packet* parse_ipv4(const u_char* packet, int packet_len) {
    // Check minimum Ethernet header
    if (packet_len < 14) {
        return nullptr;
    }

    const EthernetHdr* eth = reinterpret_cast<const EthernetHdr*>(packet);
    
    // Check if it's IPv4
    if (ntohs(eth->type) != 0x0800) {
        return nullptr;
    }

    // Check minimum IPv4 header
    if (packet_len < 34) {  // 14 (Ethernet) + 20 (min IPv4)
        return nullptr;
    }

    const IPv4Hdr* ipv4 = reinterpret_cast<const IPv4Hdr*>(packet + 14);
    
    // Get IP header length (in 32-bit words)
    int ip_header_len = (ipv4->version_length & 0x0F) * 4;
    
    if (ip_header_len < 20 || packet_len < 14 + ip_header_len) {
        return nullptr;
    }

    IPv4Packet* pkt = new IPv4Packet();
    pkt->src_ip = inet_ntoa(ipv4->src_ip);
    pkt->dst_ip = inet_ntoa(ipv4->dst_ip);
    pkt->protocol = ipv4->protocol;
    pkt->payload_size = ntohs(ipv4->total_length) - ip_header_len;

    return pkt;
}

void print_packet_info(const u_char* packet, int packet_len, const IPv4Packet* pkt) {
    static int packet_count = 0;
    packet_count++;

    const EthernetHdr* eth = reinterpret_cast<const EthernetHdr*>(packet);
    const IPv4Hdr* ipv4 = reinterpret_cast<const IPv4Hdr*>(packet + 14);
    
    int ip_header_len = (ipv4->version_length & 0x0F) * 4;
    const u_char* protocol_data = packet + 14 + ip_header_len;

    std::cout << "\n[Packet #" << packet_count << "] ";
    std::cout << "Len=" << packet_len << "B ";
    
    // Protocol name
    const char* proto_name = "Unknown";
    uint16_t src_port = 0, dst_port = 0;

    if (pkt->protocol == IPPROTO_TCP) {
        proto_name = "TCP";
        const TCPHdr* tcp = reinterpret_cast<const TCPHdr*>(protocol_data);
        src_port = ntohs(tcp->src_port);
        dst_port = ntohs(tcp->dst_port);
    } else if (pkt->protocol == IPPROTO_UDP) {
        proto_name = "UDP";
        const UDPHdr* udp = reinterpret_cast<const UDPHdr*>(protocol_data);
        src_port = ntohs(udp->src_port);
        dst_port = ntohs(udp->dst_port);
    } else if (pkt->protocol == IPPROTO_ICMP) {
        proto_name = "ICMP";
    }

    std::cout << proto_name << "\n";
    std::cout << "  Ethernet: " << format_mac(eth->src_mac) 
              << " -> " << format_mac(eth->dest_mac) << "\n";
    std::cout << "  IPv4: " << pkt->src_ip << " -> " << pkt->dst_ip;
    
    if (src_port != 0 && dst_port != 0) {
        std::cout << " [" << src_port << " -> " << dst_port << "]";
    }
    std::cout << "\n";
}

void process_packet(u_char* args, const struct pcap_pkthdr* header, const u_char* packet) {
    // Update statistics
    global_stats.increment_packets();
    global_stats.add_bytes(header->len);

    // Parse IPv4 packet
    IPv4Packet* pkt = parse_ipv4(packet, header->len);
    
    if (!pkt) {
        // Not an IPv4 packet or parsing failed
        global_stats.increment_other();
        return;
    }

    global_stats.increment_ipv4();

    // Display packet information
    print_packet_info(packet, header->len, pkt);

    delete pkt;
}
