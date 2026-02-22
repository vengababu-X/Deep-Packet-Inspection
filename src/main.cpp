#include "Sniffer.h"
#include "Stats.h"
#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "    C++ Packet Analyzer with libpcap\n";
    std::cout << "========================================\n";
    std::cout << "[*] This tool requires root/sudo privileges\n";
    std::cout << "[*] Usage: sudo ./packet_analyzer\n\n";

    start_live_capture();

    return 0;
}
