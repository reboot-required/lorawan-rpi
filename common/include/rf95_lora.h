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

/** @brief Register addresses for RF95/SX1276 in LoRa mode. */
namespace reg
{
constexpr uint8_t kFifo               = 0x00;
constexpr uint8_t kOpMode             = 0x01;
constexpr uint8_t kFrfMsb             = 0x06;
constexpr uint8_t kFrfMid             = 0x07;
constexpr uint8_t kFrfLsb             = 0x08;
constexpr uint8_t kPaConfig           = 0x09;
constexpr uint8_t kOcp                = 0x0B;
constexpr uint8_t kLna                = 0x0C;
constexpr uint8_t kFifoAddrPtr        = 0x0D;
constexpr uint8_t kFifoTxBaseAddr     = 0x0E;
constexpr uint8_t kFifoRxBaseAddr     = 0x0F;
constexpr uint8_t kFifoRxCurrentAddr  = 0x10;
constexpr uint8_t kIrqFlags           = 0x12;
constexpr uint8_t kRxNbBytes          = 0x13;
constexpr uint8_t kPktSnrValue        = 0x19;
constexpr uint8_t kPktRssiValue       = 0x1A;
constexpr uint8_t kModemConfig1       = 0x1D;
constexpr uint8_t kModemConfig2       = 0x1E;
constexpr uint8_t kPreambleMsb        = 0x20;
constexpr uint8_t kPreambleLsb        = 0x21;
constexpr uint8_t kPayloadLength      = 0x22;
constexpr uint8_t kModemConfig3       = 0x26;
constexpr uint8_t kDetectOptimize     = 0x31;
constexpr uint8_t kDetectionThreshold = 0x37;
constexpr uint8_t kSyncWord           = 0x39;
constexpr uint8_t kDioMapping1        = 0x40;
constexpr uint8_t kVersion            = 0x42;
constexpr uint8_t kPaDac              = 0x4D;
}  // namespace reg

namespace mode
{
constexpr uint8_t kLoRa         = 0x80;
constexpr uint8_t kSleep        = 0x00;
constexpr uint8_t kStandby      = 0x01;
constexpr uint8_t kTx           = 0x03;
constexpr uint8_t kRxContinuous = 0x05;
constexpr uint8_t kRxSingle     = 0x06;
}  // namespace mode

namespace irq
{
constexpr uint8_t kRxTimeout       = 0x80;
constexpr uint8_t kRxDone          = 0x40;
constexpr uint8_t kPayloadCrcError = 0x20;
constexpr uint8_t kTxDone          = 0x08;
}  // namespace irq

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

/**
 * @brief RF95/SX1276 LoRa transceiver driver.
 */
class RF95Lora
{
   public:
    /**
     * @brief Construct the driver with platform HAL objects.
     * @param spi SPI interface.
     * @param cs Chip-select GPIO.
     * @param reset Reset GPIO.
     * @param dio0 DIO0 GPIO.
     * @param delay Delay/tick interface.
     */
    RF95Lora(hal::SpiHal& spi, hal::GpioHal& cs, hal::GpioHal& reset, hal::GpioHal& dio0,
             hal::DelayHal& delay);

    /** @brief Destructor. */
    ~RF95Lora();

    /** @brief Copy construction is disabled. */
    RF95Lora(const RF95Lora&) = delete;

    /** @brief Copy assignment is disabled. */
    RF95Lora& operator=(const RF95Lora&) = delete;

    /**
     * @brief Reset chip, verify version register, and apply configuration.
     * @param config Radio configuration.
     * @return true on success, false on failure.
     */
    bool Initialize(const LoraConfig& config);

    /**
     * @brief Send one packet in blocking mode.
     * @param data Payload bytes to transmit.
     * @return true when TX completes successfully.
     */
    bool SendPacket(const std::vector<uint8_t>& data);

    /**
     * @brief Receive one packet in blocking single-RX mode.
     * @param packet Output packet structure.
     * @param timeout_ms Receive timeout in milliseconds.
     * @return true when a valid packet is received.
     */
    bool ReceivePacket(ReceivedPacket* packet, int timeout_ms = 5000);

    /**
     * @brief Enter continuous RX mode.
     */
    void StartReceive();

    /**
     * @brief Poll for a newly received packet.
     * @param packet Output packet when available.
     * @return true when a packet is available and valid.
     */
    bool CheckForPacket(ReceivedPacket* packet);

    /**
     * @brief Read transceiver version register.
     * @return Raw version register value.
     */
    uint8_t ReadVersion();

   private:
    /** @brief Perform hard reset pulse using reset GPIO. */
    void HardReset();

    /**
     * @brief Set radio operating mode.
     * @param chip_mode Mode bits from namespace mode.
     */
    void SetMode(uint8_t chip_mode);

    /**
     * @brief Configure RF center frequency.
     * @param mhz Frequency in MHz.
     */
    void ConfigureFrequency(double mhz);

    /**
     * @brief Configure TX output power.
     * @param dbm Power in dBm.
     */
    void ConfigureTxPower(int dbm);

    /**
     * @brief Configure spreading factor.
     * @param sf Spreading factor.
     */
    void ConfigureSpreadingFactor(int sf);

    /**
     * @brief Configure bandwidth.
     * @param khz Bandwidth in kHz.
     */
    void ConfigureBandwidth(int khz);

    /**
     * @brief Configure coding rate.
     * @param cr Coding rate denominator (5..8 for 4/5..4/8).
     */
    void ConfigureCodingRate(int cr);

    /**
     * @brief Configure preamble length.
     * @param symbols Number of preamble symbols.
     */
    void ConfigurePreamble(int symbols);

    /**
     * @brief Read one register over SPI.
     * @param addr Register address.
     * @return Register value.
     */
    uint8_t ReadRegister(uint8_t addr);

    /**
     * @brief Write one register over SPI.
     * @param addr Register address.
     * @param value Register value.
     */
    void WriteRegister(uint8_t addr, uint8_t value);

    /**
     * @brief Read bytes from FIFO.
     * @param buf Destination buffer.
     * @param len Number of bytes.
     */
    void ReadFifo(uint8_t* buf, uint8_t len);

    /**
     * @brief Write bytes to FIFO.
     * @param buf Source bytes.
     * @param len Number of bytes.
     */
    void WriteFifo(const uint8_t* buf, uint8_t len);

    /**
     * @brief Read and populate RSSI/SNR metrics for a packet.
     * @param pkt Output packet structure.
     */
    void ReadSignalQuality(ReceivedPacket* pkt);

    hal::SpiHal&   spi_;
    hal::GpioHal&  cs_gpio_;
    hal::GpioHal&  reset_gpio_;
    hal::GpioHal&  dio0_gpio_;
    hal::DelayHal& delay_;
    LoraConfig     config_;
    bool           initialized_ = false;
};

}  // namespace lorawan

#endif  // LORAWAN_RPI_INCLUDE_RF95_LORA_H_