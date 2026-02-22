# Packet Analyzer - Code Architecture & Design

## Overview

This is a complete C++ packet analyzer built with libpcap. It demonstrates professional software engineering practices including modular design, proper header separation, error handling, and extensibility.

## Module Breakdown

### 1. **main.cpp** - Application Entry Point

```cpp
- Initializes the application
- Displays banner and instructions
- Calls start_live_capture() from Sniffer module
- Handles program termination
```

**Key Responsibilities:**
- User interface and startup messages
- Program lifecycle management
- Exit signal handling

---

### 2. **Sniffer.cpp / Sniffer.h** - Network Interface Management

**Header (Sniffer.h):**
```cpp
void start_live_capture();
```

**Implementation (Sniffer.cpp):**
```cpp
- Enumerates all network interfaces using pcap_findalldevs()
- Filters out loopback devices (prefers real interfaces)
- Opens the first available network interface with pcap_open_live()
- Configures:
  * Buffer size: 65536 bytes (64KB snaplen)
  * Promiscuous mode: enabled (captures all traffic)
  * Read timeout: 1000ms
- Sets up SIGINT handler for graceful Ctrl+C shutdown
- Calls pcap_loop() to continuously process packets
- Hands off each packet to process_packet() from Parser
- Cleans up resources and displays final statistics
```

**Flow Diagram:**
```
pcap_findalldevs()
        ↓
    Select device
        ↓
pcap_open_live()
        ↓
pcap_loop() → process_packet() [from Parser]
        ↓
pcap_breakloop() [on Ctrl+C]
        ↓
pcap_close() + pcap_freealldevs()
```

**Why This Design:**
- ✅ Encapsulates all libpcap interaction
- ✅ Separates capture logic from parsing logic
- ✅ Error handling at each step
- ✅ Clean resource management

---

### 3. **Parser.cpp / Parser.h** - Packet Dissection

**Structures:**
```cpp
struct EthernetHdr {
    uint8_t dest_mac[6];      // Destination MAC
    uint8_t src_mac[6];       // Source MAC
    uint16_t type;            // EtherType (0x0800 = IPv4)
};

struct IPv4Hdr {
    uint8_t version_length;   // Version (4 bits) + header length (4 bits)
    uint8_t tos;              // Type of Service
    uint16_t total_length;    // Total packet length
    uint16_t id;              // Identification
    uint16_t flags_offset;    // Flags + Fragment offset
    uint8_t ttl;              // Time to Live
    uint8_t protocol;         // Protocol (6=TCP, 17=UDP, 1=ICMP)
    uint16_t checksum;        // Header checksum
    struct in_addr src_ip;    // Source IP
    struct in_addr dst_ip;    // Destination IP
};

struct TCPHdr {
    uint16_t src_port;        // Source port
    uint16_t dst_port;        // Destination port
    uint32_t seq;             // Sequence number
    uint32_t ack;             // Acknowledgment number
    uint8_t offset_reserved;  // Header offset (4 bits) + flags
    uint8_t flags;            // TCP flags (SYN, ACK, FIN, RST, etc)
    uint16_t window;          // Window size
};

struct UDPHdr {
    uint16_t src_port;        // Source port
    uint16_t dst_port;        // Destination port
    uint16_t length;          // UDP length
    uint16_t checksum;        // UDP checksum
};
```

**Main Functions:**

1. **parse_ipv4()** - Extracts IPv4 packet information
   ```cpp
   IPv4Packet* parse_ipv4(const u_char* packet, int packet_len)
   
   - Validates Ethernet frame (minimum 14 bytes)
   - Checks EtherType field for IPv4 (0x0800)
   - Parses variable-length IPv4 header
   - Extracts source/destination IP addresses
   - Returns nullptr on parse failure
   - Caller must delete returned pointer
   ```

2. **format_mac()** - Converts MAC bytes to string
   ```cpp
   std::string format_mac(const uint8_t* mac)
   
   - Converts 6 bytes to "XX:XX:XX:XX:XX:XX" format
   - Used for display only
   ```

3. **print_packet_info()** - Displays packet details
   ```cpp
   void print_packet_info(const u_char* packet, int len, const IPv4Packet* pkt)
   
   - Formats and prints packet information
   - Extracts protocol-specific details:
     * TCP: shows source/destination ports
     * UDP: shows source/destination ports
     * ICMP: shows protocol name only
   - Increments global packet counter
   ```

4. **process_packet()** - libpcap callback function
   ```cpp
   void process_packet(u_char*, const struct pcap_pkthdr*, const u_char*)
   
   SIGNATURE NOTE: Must match libpcap callback signature exactly
   
   - Called by libpcap for each captured packet
   - Updates global statistics
   - Parses IPv4 headers
   - Displays packet information
   - Cleans up memory
   ```

**Packet Layout in Memory:**
```
[Ethernet Header - 14 bytes]
  └─ Dest MAC (6) | Src MAC (6) | Type (2)

[IPv4 Header - 20+ bytes]
  └─ Version/Len (1) | ToS (1) | Total Len (2) | ID (2) | 
     Flags/Offset (2) | TTL (1) | Protocol (1) | Checksum (2) |
     Src IP (4) | Dst IP (4) | [Options (0-60)]

[TCP/UDP Header]
  └─ Src Port (2) | Dst Port (2) | [Protocol-specific fields]

[Payload Data]
  └─ Application data
```

---

### 4. **Stats.cpp / Stats.h** - Statistics Collection

**Class: Statistics**
```cpp
class Statistics {
    uint64_t total_packets = 0;    // All packet count
    uint64_t total_bytes = 0;      // All bytes count
    uint64_t ipv4_packets = 0;     // IPv4-specific count
    uint64_t other_packets = 0;    // Non-IPv4 count
};
```

**Global Instance:**
```cpp
extern Statistics global_stats;
```

**Methods:**
- `increment_packets()` - Increments total packet count
- `add_bytes(uint32_t)` - Adds to byte counter
- `increment_ipv4()` - Counts IPv4 packets
- `increment_other()` - Counts non-IPv4 packets
- `print_stats()` - Displays final statistics summary

**Statistics Output:**
```
=== CAPTURE STATISTICS ===
Total Packets: 1234
Total Bytes: 567890
IPv4 Packets: 1200
Other Packets: 34
IPv4 Percentage: 97.25%
========================
```

---

### 5. **FlowTracker.cpp / FlowTracker.h** - Flow-based Analysis

**Flow Definition:**
```cpp
struct FlowKey {
    std::string src;        // Source IP address
    std::string dst;        // Destination IP address
    int sport;              // Source port
    int dport;              // Destination port
    int proto;              // Protocol number (IPPROTO_TCP, etc)
    
    // Custom comparator for use as map key
    bool operator<(const FlowKey& other) const {
        return std::tie(src, dst, sport, dport, proto) <
               std::tie(other.src, other.dst, other.sport, other.dport, other.proto);
    }
};

struct FlowData {
    int packets = 0;        // Packets in this flow
    int bytes = 0;          // Total bytes
    std::string app;        // Application name (from DPI)
};
```

**Data Structure:**
```cpp
std::map<FlowKey, FlowData> flows;  // Ordered map of flows
```

**Methods:**
- `update(const FlowKey&, int bytes, const std::string& app)` 
  - Updates existing flow or creates new entry
  - Increments packet count and adds bytes
  - Sets application name if provided

- `print()`
  - Displays all flows in formatted table
  - Shows source → destination with port numbers
  - Displays packet/byte counts

**Example Output:**
```
=== CAPTURED FLOWS ===
Source                   Destination              Packets   Bytes     Application
----------------------------------------------------------------------
192.168.1.100:54821    -> 8.8.8.8:443              15        8245      Google
10.0.0.5:45632         -> 172.217.160.46:80       8         2156      
```

---

### 6. **DPI.cpp / DPI.h** - Application Detection

**Functions:**

1. **detect_app(domain)** - Application detection by domain
   ```cpp
   - Simple string matching in domain names
   - Case-insensitive comparison
   - Returns application name or empty string
   - Examples:
     * "www.google.com" → "Google"
     * "video.netflix.com" → "Netflix"
     * "github.com" → "GitHub"
   ```

2. **extract_tls_sni()** - TLS Server Name Indication extraction
   ```cpp
   - Simplified pattern matching in TLS handshake
   - Looks for SNI extension in ClientHello
   - Validates extracted domain name
   - NOTE: Production TLS parsing would be more complex
   ```

3. **analyze_flow()** - Combined flow analysis
   ```cpp
   - Returns DPIContext with app_name and protocol
   - Currently unused but available for future expansion
   ```

**DPIContext Structure:**
```cpp
struct DPIContext {
    std::string app_name;        // "Google", "YouTube", etc
    std::string protocol;        // "HTTP/HTTPS", "DNS", "SSH"
};
```

---

## Data Flow Diagram

```
┌─────────────────────────────────────────────────────────┐
│                   main.cpp                              │
│            Initialize & Print Banner                    │
└──────────────────┬──────────────────────────────────────┘
                   │
                   v
┌─────────────────────────────────────────────────────────┐
│           Sniffer.cpp (start_live_capture)              │
│    • Find network interfaces                             │
│    • Open pcap handle                                    │
│    • Enter pcap_loop()                                  │
└──────────────────┬──────────────────────────────────────┘
                   │ (for each packet)
                   v
┌─────────────────────────────────────────────────────────┐
│          Parser.cpp (process_packet callback)           │
│    • Validate Ethernet frame                            │
│    • Parse IPv4 header                                  │
│    • Extract TCP/UDP ports                              │
│    • Format and print output                            │
└──────────────┬─────────────┬─────────────┬──────────────┘
               │             │             │
        ┌──────v──────┐  ┌───v────┐  ┌────v─────┐
        │  Stats.cpp  │  │Flow     │  │DPI.cpp   │
        │  (update)   │  │Tracker  │  │(analyze) │
        │             │  │(update) │  │          │
        └─────────────┘  └─────────┘  └──────────┘
               │
        (Wait for Ctrl+C)
               │
        ┌──────v──────────────────────────┐
        │  Display Final Statistics:       │
        │  • Total packets & bytes         │
        │  • IPv4 vs other packets        │
        │  • All captured flows            │
        └─────────────────────────────────┘
```

---

## Memory Management

### Stack-allocated (automatic cleanup):
- Packet structures (Ethernet, IPv4, TCP, UDP headers)
- Statistical counters
- Flow keys and data

### Heap-allocated (manual cleanup):
```cpp
IPv4Packet* pkt = parse_ipv4(...);
// ... use pkt ...
delete pkt;  // IMPORTANT: Must delete!
```

### Global objects:
```cpp
extern Statistics global_stats;        // Defined in Stats.cpp
extern FlowTracker flowTracker;        // Defined in main.cpp
```

---

## Error Handling Strategy

### Packet capture failures:
- Device not found → Error message + exit
- pcap_open_live fails → Print error + exit

### Packet parsing failures:
- Invalid packet length → Return nullptr, skip packet
- Wrong EtherType → Return nullptr, count as "other"
- Malformed headers → Validate at each step

### Signal handling:
- SIGINT (Ctrl+C) → Call pcap_breakloop() for clean exit
- Display statistics on exit

---

## Compilation Model

### Makefile:
```makefile
CXX=g++                          # Compiler
CXXFLAGS=-Wall -Iinclude        # Warnings + include path
LIBS=-lpcap                     # Link against libpcap

SRC=src/main.cpp src/Sniffer.cpp ...  # Source files
OUT=packet_analyzer             # Output binary

all: $(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)
clean: rm -f $(OUT)
```

### Compilation steps:
1. Preprocess and compile each .cpp → .o object file
2. Link all objects with libpcap library
3. Create final ELF executable

---

## Extension Points

### To add DNS parsing:
```cpp
// In Parser.h
struct DNSHeader { ... };

// In Parser.cpp
void parse_dns(const u_char* packet, int port) {
    if (port != 53) return;
    // Parse DNS header and extract domain names
}
```

### To add HTTP parsing:
```cpp
// In DPI.cpp
void parse_http_request(const u_char* payload, int len) {
    // Extract HTTP method, host, user-agent
}
```

### To add flow filtering:
```cpp
// In FlowTracker.h
void filter_by_port(int port);
void filter_by_ip(const std::string& ip);
```

### To add packet export:
```cpp
// In Stats.h
void export_to_csv(const std::string& filename);
void export_to_pcap(const std::string& filename);
```

---

## Testing Approach

### Unit Testing (Manual):
```bash
# Generate ICMP traffic
ping -c 5 8.8.8.8

# Generate DNS traffic
nslookup google.com

# Generate HTTP traffic
curl -I https://example.com

# Verify output shows all packet types
```

### Integration Testing:
```bash
# Monitor specific interface
sudo ./packet_analyzer

# Generate traffic in another terminal
iperf -c 8.8.8.8
wget https://example.com
```

### Performance Testing:
```bash
# Capture for set duration and measure stats
time sudo ./packet_analyzer

# Monitor with high-speed traffic
sudo tcpdump -c 10000 | wc -l
```

---

## Conclusion

This architecture demonstrates:
- ✅ **Modularity**: Clear separation of concerns
- ✅ **Extensibility**: Easy to add new features
- ✅ **Maintainability**: Well-organized code and comments
- ✅ **Robustness**: Error handling at each layer
- ✅ **Clarity**: Meaningful variable names and structures

The codebase is ready for enhancement with additional protocol support, advanced filtering, and alternative output formats.
