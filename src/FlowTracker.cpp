#include "FlowTracker.h"
#include <iostream>
#include <iomanip>

void FlowTracker::update(const FlowKey& key, int bytes, const std::string& app) {
    auto& f = flows[key];
    f.packets++;
    f.bytes += bytes;
    if (!app.empty()) {
        f.app = app;
    }
}

void FlowTracker::print() {
    if (flows.empty()) {
        std::cout << "\n=== NO FLOWS RECORDED ===\n";
        return;
    }

    std::cout << "\n=== CAPTURED FLOWS ===\n";
    std::cout << std::left << std::setw(25) << "Source"
              << std::setw(25) << "Destination"
              << std::setw(10) << "Packets"
              << std::setw(10) << "Bytes"
              << "Application\n";
    std::cout << std::string(70, '-') << "\n";

    for (auto& [k, v] : flows) {
        std::string src = k.src + ":" + std::to_string(k.sport);
        std::string dst = k.dst + ":" + std::to_string(k.dport);
        
        std::cout << std::left << std::setw(25) << src
                  << std::setw(25) << dst
                  << std::setw(10) << v.packets
                  << std::setw(10) << v.bytes
                  << v.app << "\n";
    }
}
