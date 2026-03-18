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

- 1x Raspberry Pi (3B+/4/5/Zero 2 W)
- 1x RFM95W (868 MHz) module
- 1x matching antenna for your band
- jumper wires / breadboard

### Wiring RFM module to Raspberry Pi (BCM)

The Linux applications use `/dev/spidev0.0` for SPI chip select and request
GPIO lines `RESET=25` and `DIO0=24` through libgpiod.

```text
RFM95W                 Raspberry Pi
------                 -------------
VCC      ----------->  3.3V   (Pin 1)
GND      ----------->  GND    (Pin 6)
MISO     ----------->  GPIO 9 / SPI0_MISO (Pin 21)
MOSI     ----------->  GPIO 10 / SPI0_MOSI (Pin 19)
SCK      ----------->  GPIO 11 / SPI0_SCLK (Pin 23)
NSS/CS   ----------->  GPIO 8 / SPI0_CE0   (Pin 24)
RESET    ----------->  GPIO 25             (Pin 22)
DIO0     ----------->  GPIO 24             (Pin 18)
```

Always connect an antenna before transmitting.

## Prerequisites

Install dependencies and enable SPI:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libgpiod-dev
sudo raspi-config nonint do_spi 0
sudo reboot
```

For native x86 development builds, configuration can proceed without
`libgpiod-dev`. In that case CMake falls back to a stub GPIO backend so the
code compiles, but the Linux applications will fail during GPIO initialization
at runtime.

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

Two presets cover the two common Raspberry Pi OS variants.

### 32-bit (Pi 3B+ / Pi Zero 2 W) – `raspi-armhf`

Requires the ARMHF cross-toolchain:

```bash
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

Build:

```bash
cmake --preset raspi-armhf
cmake --build --preset raspi-armhf -j
```

Binaries:

- `build/raspi-armhf/platform_linux/lora_node`
- `build/raspi-armhf/platform_linux/lora_gateway`

### 64-bit (Pi 5 / Pi 4) – `raspi-aarch64`

Raspberry Pi OS for the Pi 5 ships as 64-bit by default. Use the `raspi-aarch64`
preset instead of `raspi-armhf` to produce a native AArch64 binary.

Requires the AArch64 cross-toolchain:

```bash
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

Build:

```bash
cmake --preset raspi-aarch64
cmake --build --preset raspi-aarch64 -j
```

Binaries:

- `build/raspi-aarch64/platform_linux/lora_node`
- `build/raspi-aarch64/platform_linux/lora_gateway`

### Optional sysroot

If you have a Raspberry Pi sysroot available for either preset, pass it during configure:

```bash
cmake --preset raspi-armhf   -DRASPI_SYSROOT=/path/to/raspi-sysroot
cmake --preset raspi-aarch64 -DRASPI_SYSROOT=/path/to/raspi-sysroot
```

### Verify output architecture

```bash
file build/raspi-armhf/platform_linux/lora_node
file build/raspi-aarch64/platform_linux/lora_node
```

## Deploy

Copy the cross-compiled binaries to the target Pi over SSH:

```bash
# Pi Zero 2 W / 32-bit OS
scp build/raspi-armhf/platform_linux/lora_node     pi@<NODE_IP>:~/
scp build/raspi-armhf/platform_linux/lora_gateway  pi@<GATEWAY_IP>:~/

# Pi 5 / 64-bit OS
scp build/raspi-aarch64/platform_linux/lora_node     pi@<NODE_IP>:~/
scp build/raspi-aarch64/platform_linux/lora_gateway  pi@<GATEWAY_IP>:~/
```

Replace `pi@<NODE_IP>` and `pi@<GATEWAY_IP>` with the actual user and IP address
of each board. After copying, run the binaries directly on the Pi as shown in the
[Run](#run) section.

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

`sudo` is typically required for `/dev/spidev*` and libgpiod-based GPIO access.

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

For this project, use only `868.1 MHz`. The applications intentionally accept
only that frequency to keep the operating profile narrow and reproducible.

This is not a substitute for verifying local regulatory requirements such as
allowed channels, duty-cycle limits, transmit power, and antenna rules before
you transmit.

## Notes

- The Linux HAL implementations use `/dev/spidev*` for SPI and `/sys/class/gpio`
  for GPIO.
- Packet tests cover serialization round-trips and CRC validation.

## License

MIT
