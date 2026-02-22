#!/bin/bash
# Demo script to showcase packet analyzer with traffic generation

echo "==========================================="
echo "   Packet Analyzer - Demo & Test Script"
echo "==========================================="
echo ""
echo "[!] This script requires root privileges"
echo "[!] Make sure the packet_analyzer is compiled"
echo ""

# Check if executable exists
if [ ! -f "./packet_analyzer" ]; then
    echo "[!] packet_analyzer not found. Please run 'make' first."
    exit 1
fi

# Run analyzer in background with timeout
echo "[*] Starting packet analyzer (will run for 30 seconds)..."
echo "[*] Open another terminal and generate traffic with:"
echo "    - ping google.com"
echo "    - curl https://example.com"
echo "    - nc example.com 80"
echo ""
echo "[*] Starting capture..."
echo ""

timeout 30 sudo ./packet_analyzer &
ANALYZER_PID=$!

# Wait a bit for the analyzer to initialize
sleep 1

# Generate some test traffic
echo "[*] Generating test traffic..."
echo ""

# Ping for ICMP traffic
ping -c 5 8.8.8.8 > /dev/null 2>&1 &
sleep 1

# Try DNS queries
nslookup google.com > /dev/null 2>&1 &
sleep 1

# Try HTTP traffic if curl is available
if command -v curl &> /dev/null; then
    timeout 3 curl -I https://www.google.com > /dev/null 2>&1 &
fi

# Wait for analyzer to finish
wait $ANALYZER_PID 2>/dev/null || true

echo ""
echo "[✓] Demo complete!"
