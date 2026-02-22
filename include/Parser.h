#ifndef PARSER_H
#define PARSER_H

#include <pcap.h>
#include <cstdint>
#include <string>

// Ethernet header
struct EthernetHeader {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t type;
};

// IPv4 header
struct IPv4Packet {
    std::string src_ip;
    std::string dst_ip;
    uint8_t protocol;
    int payload_size;
};

void process_packet(u_char*, const struct pcap_pkthdr*, const u_char*);
IPv4Packet* parse_ipv4(const u_char* packet, int packet_len);
std::string format_mac(const uint8_t* mac);

#endif
