#ifndef STATS_H
#define STATS_H

#include <cstdint>

class Statistics {
public:
    void increment_packets();
    void add_bytes(uint32_t bytes);
    void increment_ipv4();
    void increment_other();
    void print_stats();
    
    uint64_t total_packets = 0;
    uint64_t total_bytes = 0;
    uint64_t ipv4_packets = 0;
    uint64_t other_packets = 0;
};

extern Statistics global_stats;

void print_stats();

#endif