#include "Sniffer.h"
#include "Parser.h"
#include "Stats.h"
#include <pcap.h>
#include <iostream>
#include <cstring>
#include <csignal>

static pcap_t* global_handle = nullptr;

void signal_handler(int sig) {
    if (global_handle) {
        pcap_breakloop(global_handle);
    }
}

void start_live_capture() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t* devices = nullptr;
    pcap_if_t* device_to_use = nullptr;

    std::cout << "[*] Finding network devices...\n";

    if (pcap_findalldevs(&devices, errbuf) == -1) {
        std::cerr << "[!] Error finding devices: " << errbuf << "\n";
        return;
    }

    if (!devices) {
        std::cerr << "[!] No network devices found\n";
        return;
    }

    // Select first non-loopback device
    for (pcap_if_t* d = devices; d != nullptr; d = d->next) {
        if (!(d->flags & PCAP_IF_LOOPBACK)) {
            device_to_use = d;
            break;
        }
    }

    if (!device_to_use) {
        device_to_use = devices;  // fallback to first device
    }

    std::cout << "[*] Using network interface: " << device_to_use->name << "\n";
    if (device_to_use->description) {
        std::cout << "    Description: " << device_to_use->description << "\n";
    }
    std::cout << "[*] Starting packet capture (Ctrl+C to stop)...\n\n";

    // Open device for live capture
    global_handle = pcap_open_live(
        device_to_use->name,    // device name
        65536,                   // snaplen (max bytes per packet)
        1,                       // promiscuous mode
        1000,                    // read timeout (ms)
        errbuf                   // error buffer
    );

    if (!global_handle) {
        std::cerr << "[!] pcap_open_live() failed: " << errbuf << "\n";
        std::cerr << "[!] Make sure you run with sudo/root privileges\n";
        pcap_freealldevs(devices);
        return;
    }

    // Set up signal handler for clean exit
    signal(SIGINT, signal_handler);

    // Start packet loop (0 = unlimited packets)
    pcap_loop(global_handle, 0, process_packet, nullptr);

    std::cout << "\n[*] Capture stopped\n";

    // Cleanup
    pcap_close(global_handle);
    pcap_freealldevs(devices);
    
    global_stats.print_stats();
}
