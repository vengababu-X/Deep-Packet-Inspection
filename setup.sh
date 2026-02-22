#!/bin/bash
# Quick setup script for packet analyzer on Linux/WSL

set -e  # Exit on error

echo "=================================="
echo "  Packet Analyzer Setup Script"
echo "=================================="

# Check if running as root
if [ "$(id -u)" != "0" ]; then
    echo "[!] This script should be run with 'sudo' for package installation"
    echo "[!] Proceeding with what we can do..."
fi

# Detect OS
if [-f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
else
    OS="Unknown"
fi

echo "[*] Detected OS: $OS"

# Install dependencies
if command -v apt-get &> /dev/null; then
    echo "[*] Installing dependencies with apt..."
    sudo apt-get update
    sudo apt-get install -y build-essential libpcap-dev
elif command -v yum &> /dev/null; then
    echo "[*] Installing dependencies with yum..."
    sudo yum groupinstall -y "Development Tools"
    sudo yum install -y libpcap-devel
elif command -v brew &> /dev/null; then
    echo "[*] Installing dependencies with brew..."
    brew install libpcap
else
    echo "[!] Could not find package manager. Please install manually:"
    echo "    - gcc/g++ compiler"
    echo "    - libpcap development headers"
    exit 1
fi

# Build the project
echo "[*] Building packet analyzer..."
cd "$(dirname "$0")"
make clean
make

echo "[✓] Build successful!"
echo ""
echo "Quick start:"
echo "  sudo ./packet_analyzer"
echo ""
echo "To stop: Press Ctrl+C"
