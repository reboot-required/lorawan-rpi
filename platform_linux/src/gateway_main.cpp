// gateway_main.cpp: LoRa gateway application entry point.

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "linux_delay.h"
#include "linux_gpio.h"
#include "linux_spi.h"
#include "logger.h"
#include "lora_packet.h"
#include "radio_profile.h"
#include "rf95_lora.h"

volatile std::sig_atomic_t g_running = 1;

void HandleSignal(int /*sig*/) { g_running = 0; }

int main(int argc, char* argv[])
{
    std::signal(SIGINT, HandleSignal);
    std::signal(SIGTERM, HandleSignal);

    // --- Defaults ---
    double      frequency        = lorawan::radio_profile::kOnlyFrequencyMhz;
    std::string spi_path         = "/dev/spidev0.0";
    int         spreading_factor = 7;

    for (int i = 1; i < argc - 1; i += 2)
    {
        std::string key = argv[i];
        std::string val = argv[i + 1];
        if (key == "--freq")
        {
            frequency = std::stod(val);
        }
        else if (key == "--spi")
        {
            spi_path = val;
        }
        else if (key == "--sf")
        {
            spreading_factor = std::stoi(val);
        }
    }

    if (!lorawan::radio_profile::IsAllowedFrequency(frequency))
    {
        lorawan::Logger::Error("Only 868.1 MHz is supported. Use --freq 868.1");
        return 1;
    }

    lorawan::Logger::SetLevel(lorawan::LogLevel::kDebug);
    lorawan::Logger::Info("=== LoRa Gateway ===");

    // --- Hardware init ---
    lorawan::rpi_linux::LinuxSpi spi(spi_path);

    lorawan::rpi_linux::LinuxGpio gpio_cs(8, true);
    lorawan::rpi_linux::LinuxGpio gpio_reset(25, true);
    lorawan::rpi_linux::LinuxGpio gpio_dio0(24, false);

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
    cfg.tx_power_dbm     = lorawan::radio_profile::kGatewayDefaultPowerDbm;
    cfg.sync_word        = static_cast<uint8_t>(lorawan::radio_profile::kDefaultSyncWord);
    cfg.crc_enabled      = lorawan::radio_profile::kDefaultCrcEnabled;

    if (!gpio_cs.Init() || !gpio_reset.Init() || !gpio_dio0.Init())
    {
        lorawan::Logger::Error("GPIO init failed - aborting");
        return 1;
    }

    if (!radio.Initialize(cfg))
    {
        lorawan::Logger::Error("RF95 init failed – aborting");
        return 1;
    }

    // --- Enter continuous receive mode ---
    radio.StartReceive();
    lorawan::Logger::Info("Listening on " + std::to_string(frequency) + " MHz  (SF" +
                          std::to_string(spreading_factor) + ", BW125)  – Ctrl+C to stop");

    uint32_t rx_count = 0;

    while (g_running != 0)
    {
        lorawan::ReceivedPacket rx;
        if (radio.CheckForPacket(&rx))
        {
            ++rx_count;

            lorawan::Logger::Info("──── Packet #" + std::to_string(rx_count) + " ────");
            lorawan::Logger::Info("  RSSI : " + std::to_string(rx.rssi) + " dBm");
            lorawan::Logger::Info("  SNR  : " + std::to_string(rx.snr) + " dB");
            lorawan::Logger::Info("  Bytes: " + std::to_string(rx.payload.size()));

            // Try to decode as our LoraPacket format.
            lorawan::LoraPacket pkt;
            if (lorawan::LoraPacket::Deserialize(rx.payload, &pkt))
            {
                lorawan::Logger::Info("  " + pkt.ToString());

                // Print payload as ASCII if printable.
                bool printable = true;
                for (uint8_t c : pkt.payload)
                {
                    if (c < 0x20 || c > 0x7E)
                    {
                        printable = false;
                        break;
                    }
                }
                if (printable && !pkt.payload.empty())
                {
                    std::string text(pkt.payload.begin(), pkt.payload.end());
                    lorawan::Logger::Info("  Text : \"" + text + "\"");
                }
            }
            else
            {
                lorawan::Logger::Warning("  CRC mismatch – raw dump only");
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    lorawan::Logger::Info("Gateway stopped (received " + std::to_string(rx_count) + " packets)");
    return 0;
}