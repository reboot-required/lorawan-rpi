// node_main.cpp: LoRa node application entry point.

#include <csignal>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "linux_delay.h"
#include "linux_gpio.h"
#include "linux_spi.h"
#include "logger.h"
#include "lora_packet.h"
#include "radio_profile.h"
#include "rf95_lora.h"

namespace
{

volatile std::sig_atomic_t g_running = 1;

void HandleSignal(int /*sig*/) { g_running = 0; }

void PrintUsage(const char* prog)
{
    std::cout << "Usage: " << prog
              << " [--addr HEX] [--freq 868.1] [--interval SEC]"
                 " [--spi DEV] [--sf N] [--power DBM]\n";
}

}  // namespace

int main(int argc, char* argv[])
{
    std::signal(SIGINT, HandleSignal);
    std::signal(SIGTERM, HandleSignal);

    // --- Defaults ---
    uint32_t    device_address   = 0x01000001;
    double      frequency        = lorawan::radio_profile::kOnlyFrequencyMhz;
    int         interval_sec     = 10;
    std::string spi_path         = "/dev/spidev0.0";
    int         spreading_factor = 7;
    int         tx_power         = 17;

    // --- Parse CLI args ---
    for (int i = 1; i < argc - 1; i += 2)
    {
        std::string key = argv[i];
        std::string val = argv[i + 1];
        if (key == "--addr")
        {
            device_address = static_cast<uint32_t>(std::stoul(val, nullptr, 16));
        }
        else if (key == "--freq")
        {
            frequency = std::stod(val);
        }
        else if (key == "--interval")
        {
            interval_sec = std::stoi(val);
        }
        else if (key == "--spi")
        {
            spi_path = val;
        }
        else if (key == "--sf")
        {
            spreading_factor = std::stoi(val);
        }
        else if (key == "--power")
        {
            tx_power = std::stoi(val);
        }
        else if (key == "--help")
        {
            PrintUsage(argv[0]);
            return 0;
        }
    }

    if (!lorawan::radio_profile::IsAllowedFrequency(frequency))
    {
        lorawan::Logger::Error("Only 868.1 MHz is supported. Use --freq 868.1");
        return 1;
    }

    lorawan::Logger::SetLevel(lorawan::LogLevel::kDebug);
    lorawan::Logger::Info("=== LoRa Node ===");

    // --- Hardware init ---
    lorawan::rpi_linux::LinuxSpi   spi(spi_path);
    lorawan::rpi_linux::LinuxGpio  gpio_cs(8, true);
    lorawan::rpi_linux::LinuxGpio  gpio_reset(25, true);
    lorawan::rpi_linux::LinuxGpio  gpio_dio0(24, false);
    lorawan::rpi_linux::LinuxDelay delay;

    if (!spi.Open())
    {
        lorawan::Logger::Error("SPI open failed – aborting");
        return 1;
    }

    lorawan::RF95Lora radio(spi, gpio_cs, gpio_reset, gpio_dio0, delay);

    lorawan::LoraConfig cfg;
    cfg.frequency_mhz    = frequency;
    cfg.spreading_factor = spreading_factor;
    cfg.bandwidth_khz    = lorawan::radio_profile::kDefaultBandwidthKhz;
    cfg.coding_rate      = lorawan::radio_profile::kDefaultCodingRate;
    cfg.tx_power_dbm     = tx_power;
    cfg.sync_word        = static_cast<uint8_t>(lorawan::radio_profile::kDefaultSyncWord);
    cfg.crc_enabled      = lorawan::radio_profile::kDefaultCrcEnabled;

    if (!radio.Initialize(cfg))
    {
        lorawan::Logger::Error("RF95 init failed – aborting");
        return 1;
    }

    // --- Main loop: build packet → send → sleep → repeat ---
    uint16_t frame_counter = 0;

    while (g_running != 0)
    {
        lorawan::LoraPacket pkt;
        pkt.device_address = device_address;
        pkt.frame_counter  = frame_counter;
        pkt.port           = 1;

        std::string msg = "Hello #" + std::to_string(frame_counter);
        pkt.payload.assign(msg.begin(), msg.end());

        lorawan::Logger::Info("TX " + pkt.ToString());

        std::vector<uint8_t> raw = pkt.Serialize();
        if (radio.SendPacket(raw))
        {
            lorawan::Logger::Info("TX OK (" + std::to_string(raw.size()) + " bytes on air)");
        }
        else
        {
            lorawan::Logger::Error("TX FAILED");
        }

        ++frame_counter;

        // Interruptible sleep.
        for (int s = 0; s < interval_sec && g_running != 0; ++s)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    lorawan::Logger::Info("Node stopped (sent " + std::to_string(frame_counter) + " packets)");
    return 0;
}