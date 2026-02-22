#include "Stats.h"
#include <iostream>
#include <iomanip>

Statistics global_stats;

void Statistics::increment_packets() {
    total_packets++;
}

void Statistics::add_bytes(uint32_t bytes) {
    total_bytes += bytes;
}

void Statistics::increment_ipv4() {
    ipv4_packets++;
}

void Statistics::increment_other() {
    other_packets++;
}

void Statistics::print_stats() {
    std::cout << "\n=== CAPTURE STATISTICS ===\n";
    std::cout << "Total Packets: " << total_packets << "\n";
    std::cout << "Total Bytes: " << total_bytes << "\n";
    std::cout << "IPv4 Packets: " << ipv4_packets << "\n";
    std::cout << "Other Packets: " << other_packets << "\n";
    
    if (total_packets > 0) {
        double ipv4_percent = (static_cast<double>(ipv4_packets) / total_packets) * 100;
        std::cout << "IPv4 Percentage: " << std::fixed << std::setprecision(2) 
                  << ipv4_percent << "%\n";
    }
    std::cout << "========================\n";
}

void print_stats() {
    global_stats.print_stats();
}
