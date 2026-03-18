/**
 * @file rf95_lora.h
 * @brief RF95/SX1276 LoRa driver API.
 */

#ifndef LORAWAN_RPI_INCLUDE_RF95_LORA_H_
#define LORAWAN_RPI_INCLUDE_RF95_LORA_H_

#include <cstdint>
#include <vector>

#include "hal/delay_hal.h"
#include "hal/gpio_hal.h"
#include "hal/spi_hal.h"

namespace lorawan
{

/**
 * @brief Runtime radio configuration for RF95Lora.
 */
struct LoraConfig
{
    double  frequency_mhz    = 868.1;  // RF95 = 868 MHz band
    int     spreading_factor = 7;      // 6 – 12
    int     bandwidth_khz    = 125;    // 7.8 … 500
    int     coding_rate      = 5;      // 5 – 8  (= 4/5 … 4/8)
    int     tx_power_dbm     = 17;     // 2 – 20
    int     preamble_length  = 8;
    uint8_t sync_word        = 0x34;  // 0x34 = LoRaWAN public
    bool    crc_enabled      = true;
    bool    implicit_header  = false;
};

/**
 * @brief Container for a received packet and link metrics.
 */
struct ReceivedPacket
{
    std::vector<uint8_t> payload;
    int                  rssi = 0;     // dBm
    float                snr  = 0.0f;  // dB
};

/** @brief RF95/SX1276 LoRa transceiver driver. */
class RF95Lora
{
   public:
    RF95Lora(hal::SpiHal& spi, hal::GpioHal& reset, hal::GpioHal& dio0, hal::DelayHal& delay);
    ~RF95Lora();
    RF95Lora(const RF95Lora&)            = delete;
    RF95Lora& operator=(const RF95Lora&) = delete;

    /** @brief Reset chip, verify version register, and apply configuration. */
    bool Initialize(const LoraConfig& config);

    /** @brief Send one packet in blocking mode. */
    bool SendPacket(const std::vector<uint8_t>& data);

    /**
     * @brief Receive one packet in blocking single-RX mode.
     * @param timeout_ms Receive timeout in milliseconds (default 5000).
     * @return true when a valid packet is received.
     */
    bool ReceivePacket(ReceivedPacket* packet, int timeout_ms = 5000);

    /** @brief Enter continuous RX mode. */
    void StartReceive();

    /** @brief Poll for a newly received packet (use after StartReceive). */
    bool CheckForPacket(ReceivedPacket* packet);

    /** @brief Read transceiver version register (0x12 expected). */
    uint8_t ReadVersion();

   private:
    void    SetMode(uint8_t chip_mode);
    void    ConfigureFrequency(double mhz);
    void    ConfigureTxPower(int dbm);
    void    ConfigureSpreadingFactor(int sf);
    void    ConfigureBandwidth(int khz);
    void    ConfigureCodingRate(int cr);
    void    ConfigurePreamble(int symbols);
    uint8_t ReadRegister(uint8_t addr);
    void    WriteRegister(uint8_t addr, uint8_t value);
    bool    ReadFifo(uint8_t* buf, uint8_t len);
    bool    WriteFifo(const uint8_t* buf, uint8_t len);
    void    ReadSignalQuality(ReceivedPacket* pkt);

    hal::SpiHal&   spi_;
    hal::GpioHal&  reset_gpio_;
    hal::GpioHal&  dio0_gpio_;
    hal::DelayHal& delay_;
    LoraConfig     config_;
    bool           initialized_ = false;
};

}  // namespace lorawan

#endif  // LORAWAN_RPI_INCLUDE_RF95_LORA_H_