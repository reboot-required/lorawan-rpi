#!/bin/bash
# setup.sh: Raspberry Pi environment setup for lorawan-rpi.

set -euo pipefail

echo "======================================="
echo " LoRa RPi - System Setup"
echo "======================================="

if [ "$EUID" -ne 0 ]; then
  echo "Error: please run with sudo."
  exit 1
fi

echo "[1/3] Enabling SPI..."
raspi-config nonint do_spi 0

echo "[2/3] Installing build tools..."
apt-get update -qq
apt-get install -y -qq build-essential cmake

echo "[3/3] Checking SPI device..."
if [ -e /dev/spidev0.0 ]; then
  echo "  OK: /dev/spidev0.0 found"
else
  echo "  WARNING: /dev/spidev0.0 not found."
  echo "  Please check again after reboot."
fi

echo ""
echo "Setup complete. Please reboot:"
echo "  sudo reboot"
echo ""
echo "Then build with:"
echo "  cmake -S . -B build && cmake --build build -j"