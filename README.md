# LoRa RPi - Embedded Linux LoRa Communication

Minimal LoRa packet communication on Raspberry Pi using RF95-style
modules and C++17.

This repository contains a small LoRaWAN-inspired packet layer
(DevAddr/FrameCnt/Port/Payload/CRC16), a radio driver, and two Linux apps:
`lora_node` (TX) and `lora_gateway` (RX).

## Overview

```text
┌──────────────┐           LoRa 868 MHz            ┌──────────────┐
│ Raspberry Pi │  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  │ Raspberry Pi │
│   (Node)     │ ──── RFM95 + antenna ──────────── │  (Gateway)   │
└──────────────┘                                   └──────────────┘
```

- `lora_node`: creates packets periodically and sends them over LoRa.
- `lora_gateway`: listens continuously, validates packets, prints RSSI/SNR.

## Repository Layout

```text
lorawan-rpi/
├── CMakeLists.txt
├── setup.sh
├── common/
│   ├── include/
│   │   ├── hal/{spi_hal.h,gpio_hal.h,delay_hal.h}
│   │   ├── rf95_lora.h
│   │   ├── lora_packet.h
│   │   └── logger.h
│   └── src/
│       ├── rf95_lora.cpp
│       ├── lora_packet.cpp
│       └── logger.cpp
└── platform_linux/
    ├── CMakeLists.txt
    ├── include/{linux_spi.h,linux_gpio.h,linux_delay.h}
    └── src/
        ├── node_main.cpp
        ├── gateway_main.cpp
        ├── linux_spi.cpp
        ├── linux_gpio.cpp
        └── linux_delay.cpp
```

## Hardware

### Components (per side)

- 1x Raspberry Pi (3B+/4/5/Zero 2W)
- 1x RFM95W (868 MHz) module
- 1x matching antenna for your band
- jumper wires / breadboard

### Wiring RFM module to Raspberry Pi (BCM)

```text
RFM module       Raspberry Pi
VCC              3.3V (Pin 1)
GND              GND  (Pin 6)
MISO             GPIO 9  / SPI0_MISO (Pin 21)
MOSI             GPIO 10 / SPI0_MOSI (Pin 19)
SCK              GPIO 11 / SPI0_SCLK (Pin 23)
NSS/CS           GPIO 8  / SPI0_CE0  (Pin 24)
RESET            GPIO 25             (Pin 22)
DIO0             GPIO 24             (Pin 18)
```

Always connect an antenna before transmitting.

## Prerequisites

Install dependencies and enable SPI:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake
sudo raspi-config nonint do_spi 0
sudo reboot
```

Or run the project setup script:

```bash
sudo bash ./setup.sh
sudo reboot
```

## Build

This project builds with CMake from the repository root:

```bash
cmake -S . -B build
cmake --build build -j
```

Preset-based builds are also supported:

```bash
cmake --preset host-debug
cmake --build --preset host-debug -j
```

Binaries are generated at:

- `build/platform_linux/lora_node`
- `build/platform_linux/lora_gateway`

Build only one target:

```bash
cmake --build build --target lora_node -j
cmake --build build --target lora_gateway -j
```

Clean build directory:

```bash
rm -rf build
```

## Cross-Compile For Raspberry Pi

Use the dedicated `raspi-armhf` preset to produce 32-bit Raspberry Pi Linux binaries
from an x86 Linux host with the ARMHF cross-toolchain installed:

```bash
cmake --preset raspi-armhf
cmake --build --preset raspi-armhf -j
```

Cross-compiled binaries are generated at:

- `build/raspi-armhf/platform_linux/lora_node`
- `build/raspi-armhf/platform_linux/lora_gateway`

The preset uses `cmake/toolchains/raspi-armhf.cmake` and expects:

- `arm-linux-gnueabihf-gcc`
- `arm-linux-gnueabihf-g++`

If you also have a Raspberry Pi sysroot available, pass it during configure:

```bash
cmake --preset raspi-armhf -DRASPI_SYSROOT=/path/to/raspi-sysroot
```

To confirm the output architecture:

```bash
file build/raspi-armhf/platform_linux/lora_node
file build/raspi-armhf/platform_linux/lora_gateway
```

## Test

Build and run CTest from the same build directory:

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

`lora_packet_tests` validates packet round-trip serialization and CRC checks.
When using an ARM cross-toolchain on a non-ARM host, runtime test execution is
skipped and a placeholder CTest case is reported instead.

## Run

Gateway (receiver):

```bash
sudo ./build/platform_linux/lora_gateway --spi /dev/spidev0.0 --sf 7
```

Node (sender):

```bash
sudo ./build/platform_linux/lora_node --addr 01000001 --interval 10 --spi /dev/spidev0.0 --sf 7 --power 17
```

`sudo` is typically required for `/dev/spidev*` and GPIO access.

## Command-Line Options

`lora_node` supports:

- `--addr HEX` (example: `01000001`)
- `--freq 868.1` (only allowed value)
- `--interval SEC`
- `--spi DEV`
- `--sf N`
- `--power DBM`

`lora_gateway` supports:

- `--freq 868.1` (only allowed value)
- `--spi DEV`
- `--sf N`

## Packet Format

```text
+----------+----------+------+---------+-------+
| DevAddr  | FrameCnt | Port | Payload | CRC16 |
| 4 bytes  | 2 bytes  | 1 B  | N bytes | 2 B   |
+----------+----------+------+---------+-------+
```

- multi-byte fields are big-endian
- CRC is CRC16-CCITT over header + payload

## LoRa Defaults In Code

- Node defaults: `freq=868.1`, `sf=7`, `bw=125`, `cr=4/5`, `power=17`, `sync=0x34`
- Gateway defaults: `freq=868.1`, `sf=7`, `bw=125`, `cr=4/5`, `power=14`, `sync=0x34`

Frequency policy: this project enforces `868.1 MHz` only. If another
frequency is passed via `--freq`, the application exits with an error.

## Regulatory Note (Germany)

For this project, use only `868.1 MHz`. The implementation and examples are
intentionally restricted to `868.1 MHz`, and in Germany this is the only
frequency intended/allowed for operation in this repository.

## Notes

- `README.md` is now aligned with the current codebase (Linux + CMake).
- The previous `README.md` content describing ESP32 and `scripts/setup_rpi.sh`
  does not match files present in this repository.

## License

MIT
