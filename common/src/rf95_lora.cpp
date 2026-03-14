// rf95_lora.cpp: RF95/SX1276 LoRa driver implementation.

#include "rf95_lora.h"

#include <chrono>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <thread>

#include "logger.h"

namespace lorawan
{
namespace
{

std::string Hex8(uint8_t v)
{
    std::ostringstream o;
    o << "0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(v);
    return o.str();
}

}  // namespace

// --------------------------------------------------------------------
// Construction / Destruction
// --------------------------------------------------------------------

RF95Lora::RF95Lora(hal::SpiHal& spi, hal::GpioHal& cs, hal::GpioHal& reset, hal::GpioHal& dio0,
                   hal::DelayHal& delay)
    : spi_(spi), cs_gpio_(cs), reset_gpio_(reset), dio0_gpio_(dio0), delay_(delay)
{
}

RF95Lora::~RF95Lora()
{
    if (initialized_)
    {
        SetMode(mode::kSleep);
    }
}

// --------------------------------------------------------------------
// Public API
// --------------------------------------------------------------------

bool RF95Lora::Initialize(const LoraConfig& config)
{
    reset_gpio_.Init();
    cs_gpio_.Init();
    dio0_gpio_.Init();

    reset_gpio_.SetLow();
    delay_.DelayMs(10);

    reset_gpio_.SetHigh();
    delay_.DelayMs(10);

    // --- Verify chip ID -----------------------------------------------
    uint8_t version = ReadVersion();
    if (version != 0x12)
    {
        Logger::Error("RF95 not detected (version reg = " + Hex8(version) + ", expected 0x12)");
        return false;
    }
    Logger::Info("RF95/SX1276 detected (version " + Hex8(version) + ")");

    // --- Switch to LoRa sleep mode ------------------------------------
    WriteRegister(reg::kOpMode, mode::kLoRa | mode::kSleep);
    delay_.DelayMs(10);

    // --- Standby mode -------------------------------------------------
    SetMode(mode::kStandby);
    delay_.DelayMs(10);

    // --- Apply radio parameters ---------------------------------------
    ConfigureFrequency(config_.frequency_mhz);
    ConfigureTxPower(config_.tx_power_dbm);
    ConfigureSpreadingFactor(config_.spreading_factor);
    ConfigureBandwidth(config_.bandwidth_khz);
    ConfigureCodingRate(config_.coding_rate);
    ConfigurePreamble(config_.preamble_length);

    // Sync word
    WriteRegister(reg::kSyncWord, config_.sync_word);

    // CRC
    uint8_t mc2 = ReadRegister(reg::kModemConfig2);
    if (config_.crc_enabled)
    {
        mc2 |= 0x04;
    }
    else
    {
        mc2 &= ~0x04;
    }
    WriteRegister(reg::kModemConfig2, mc2);

    // Implicit / explicit header
    uint8_t mc1 = ReadRegister(reg::kModemConfig1);
    if (config_.implicit_header)
    {
        mc1 |= 0x01;
    }
    else
    {
        mc1 &= ~0x01;
    }
    WriteRegister(reg::kModemConfig1, mc1);

    // FIFO base addresses
    WriteRegister(reg::kFifoTxBaseAddr, 0x00);
    WriteRegister(reg::kFifoRxBaseAddr, 0x00);

    // LNA boost (max gain)
    WriteRegister(reg::kLna, ReadRegister(reg::kLna) | 0x03);

    // AGC auto on
    WriteRegister(reg::kModemConfig3, 0x04);

    initialized_ = true;
    Logger::Info("RF95 initialised – " + std::to_string(config_.frequency_mhz) + " MHz, SF" +
                 std::to_string(config_.spreading_factor) + ", BW" +
                 std::to_string(config_.bandwidth_khz) + " kHz");
    return true;
}

bool RF95Lora::SendPacket(const std::vector<uint8_t>& data)
{
    if (!initialized_ || data.empty() || data.size() > 255)
    {
        return false;
    }

    SetMode(mode::kStandby);

    // Point FIFO to TX base and write payload.
    WriteRegister(reg::kFifoAddrPtr, 0x00);
    WriteFifo(data.data(), static_cast<uint8_t>(data.size()));
    WriteRegister(reg::kPayloadLength, static_cast<uint8_t>(data.size()));

    // Clear all IRQ flags, then start TX.
    WriteRegister(reg::kIrqFlags, 0xFF);
    SetMode(mode::kTx);

    // Wait for TxDone (poll IRQ-flags register).
    uint16_t timeout_counter = 0;
    while (true)
    {
        uint8_t flags = ReadRegister(reg::kIrqFlags);
        if (flags & irq::kTxDone)
        {
            break;
        }
        if (timeout_counter > 1000)
        {
            Logger::Error("SendPacket: TX timeout");
            SetMode(mode::kStandby);
            return false;
        }
        timeout_counter++;
        delay_.DelayMs(10);
    }

    WriteRegister(reg::kIrqFlags, 0xFF);
    SetMode(mode::kStandby);
    return true;
}

bool RF95Lora::ReceivePacket(ReceivedPacket* packet, int timeout_ms)
{
    if (!initialized_ || packet == nullptr)
    {
        return false;
    }

    SetMode(mode::kStandby);
    WriteRegister(reg::kIrqFlags, 0xFF);
    WriteRegister(reg::kFifoAddrPtr, ReadRegister(reg::kFifoRxBaseAddr));
    SetMode(mode::kRxSingle);

    // auto t0 = std::chrono::steady_clock::now();
    while (true)
    {
        uint8_t flags = ReadRegister(reg::kIrqFlags);

        if (flags & irq::kRxDone)
        {
            if (flags & irq::kPayloadCrcError)
            {
                Logger::Warning("ReceivePacket: HW CRC error");
                WriteRegister(reg::kIrqFlags, 0xFF);
                return false;
            }

            uint8_t len = ReadRegister(reg::kRxNbBytes);
            WriteRegister(reg::kFifoAddrPtr, ReadRegister(reg::kFifoRxCurrentAddr));
            packet->payload.resize(len);
            ReadFifo(packet->payload.data(), len);
            ReadSignalQuality(packet);
            WriteRegister(reg::kIrqFlags, 0xFF);
            return true;
        }

        if (flags & irq::kRxTimeout)
        {
            WriteRegister(reg::kIrqFlags, 0xFF);
            return false;
        }

        // auto dt = std::chrono::steady_clock::now() - t0;
        // if (std::chrono::duration_cast<std::chrono::milliseconds>(dt).count() > timeout_ms)
        // {
        //     SetMode(mode::kStandby);
        //     return false;
        // }
        delay_.DelayMs(1);
    }
}

void RF95Lora::StartReceive()
{
    if (!initialized_)
    {
        return;
    }
    WriteRegister(reg::kIrqFlags, 0xFF);
    WriteRegister(reg::kFifoAddrPtr, ReadRegister(reg::kFifoRxBaseAddr));
    // Map DIO0 → RxDone.
    WriteRegister(reg::kDioMapping1, 0x00);
    SetMode(mode::kRxContinuous);
}

bool RF95Lora::CheckForPacket(ReceivedPacket* packet)
{
    if (!initialized_ || packet == nullptr)
    {
        return false;
    }

    uint8_t flags = ReadRegister(reg::kIrqFlags);
    if (!(flags & irq::kRxDone))
    {
        return false;
    }

    if (flags & irq::kPayloadCrcError)
    {
        Logger::Warning("CheckForPacket: HW CRC error");
        WriteRegister(reg::kIrqFlags, 0xFF);
        return false;
    }

    uint8_t len = ReadRegister(reg::kRxNbBytes);
    WriteRegister(reg::kFifoAddrPtr, ReadRegister(reg::kFifoRxCurrentAddr));

    packet->payload.resize(len);
    ReadFifo(packet->payload.data(), len);
    ReadSignalQuality(packet);

    // Clear flags and stay in RX-continuous.
    WriteRegister(reg::kIrqFlags, 0xFF);
    return true;
}

uint8_t RF95Lora::ReadVersion() { return ReadRegister(reg::kVersion); }

// --------------------------------------------------------------------
// Private helpers
// --------------------------------------------------------------------

void RF95Lora::HardReset()
{
    if (!reset_gpio_.Read())
    {
        return;
    }
    reset_gpio_.SetLow();
    delay_.DelayMs(10);
    reset_gpio_.SetHigh();
    delay_.DelayMs(10);
}

void RF95Lora::SetMode(uint8_t chip_mode) { WriteRegister(reg::kOpMode, mode::kLoRa | chip_mode); }

void RF95Lora::ConfigureFrequency(double mhz)
{
    // frf = freq_hz * 2^19 / F_XOSC   (F_XOSC = 32 MHz)
    uint64_t frf = static_cast<uint64_t>((mhz * 1.0e6 * (1 << 19)) / 32.0e6);
    WriteRegister(reg::kFrfMsb, static_cast<uint8_t>((frf >> 16) & 0xFF));
    WriteRegister(reg::kFrfMid, static_cast<uint8_t>((frf >> 8) & 0xFF));
    WriteRegister(reg::kFrfLsb, static_cast<uint8_t>((frf) & 0xFF));
}

void RF95Lora::ConfigureTxPower(int dbm)
{
    if (dbm < 2) dbm = 2;
    if (dbm > 20) dbm = 20;

    uint8_t output_power;
    if (dbm <= 17)
    {
        WriteRegister(reg::kPaDac, 0x84);  // default PA
        output_power = static_cast<uint8_t>(dbm - 2);
    }
    else
    {
        WriteRegister(reg::kPaDac, 0x87);  // +20 dBm mode
        WriteRegister(reg::kOcp, 0x3F);    // OCP 240 mA
        output_power = static_cast<uint8_t>(dbm - 5);
    }
    // PaSelect = 1 (PA_BOOST pin)
    WriteRegister(reg::kPaConfig, 0x80 | (output_power & 0x0F));
}

void RF95Lora::ConfigureSpreadingFactor(int sf)
{
    if (sf < 6) sf = 6;
    if (sf > 12) sf = 12;

    uint8_t mc2 = ReadRegister(reg::kModemConfig2);
    mc2         = (mc2 & 0x0F) | (static_cast<uint8_t>(sf) << 4);
    WriteRegister(reg::kModemConfig2, mc2);

    // SF6 requires specific DetectOptimize / DetectionThreshold values.
    if (sf == 6)
    {
        WriteRegister(reg::kDetectOptimize, 0xC5);
        WriteRegister(reg::kDetectionThreshold, 0x0C);
    }
    else
    {
        WriteRegister(reg::kDetectOptimize, 0xC3);
        WriteRegister(reg::kDetectionThreshold, 0x0A);
    }

    // Low-data-rate optimisation (mandatory when symbol time > 16 ms).
    if (sf >= 11 && config_.bandwidth_khz <= 125)
    {
        uint8_t mc3 = ReadRegister(reg::kModemConfig3);
        mc3 |= 0x08;
        WriteRegister(reg::kModemConfig3, mc3);
    }
}

void RF95Lora::ConfigureBandwidth(int khz)
{
    uint8_t bw;
    if (khz <= 8)
        bw = 0;  //   7.8 kHz
    else if (khz <= 11)
        bw = 1;  //  10.4 kHz
    else if (khz <= 16)
        bw = 2;  //  15.6 kHz
    else if (khz <= 21)
        bw = 3;  //  20.8 kHz
    else if (khz <= 32)
        bw = 4;  //  31.25 kHz
    else if (khz <= 42)
        bw = 5;  //  41.7 kHz
    else if (khz <= 63)
        bw = 6;  //  62.5 kHz
    else if (khz <= 125)
        bw = 7;  // 125   kHz
    else if (khz <= 250)
        bw = 8;  // 250   kHz
    else
        bw = 9;  // 500   kHz

    uint8_t mc1 = ReadRegister(reg::kModemConfig1);
    mc1         = (mc1 & 0x0F) | (bw << 4);
    WriteRegister(reg::kModemConfig1, mc1);
}

void RF95Lora::ConfigureCodingRate(int cr)
{
    if (cr < 5) cr = 5;
    if (cr > 8) cr = 8;
    uint8_t cr_val = static_cast<uint8_t>(cr - 4);  // 1–4
    uint8_t mc1    = ReadRegister(reg::kModemConfig1);
    mc1            = (mc1 & 0xF1) | (cr_val << 1);
    WriteRegister(reg::kModemConfig1, mc1);
}

void RF95Lora::ConfigurePreamble(int symbols)
{
    WriteRegister(reg::kPreambleMsb, static_cast<uint8_t>((symbols >> 8) & 0xFF));
    WriteRegister(reg::kPreambleLsb, static_cast<uint8_t>(symbols & 0xFF));
}

// ---- Low-level SPI register access ------------------------------------

uint8_t RF95Lora::ReadRegister(uint8_t addr)
{
    uint8_t tx[2] = {static_cast<uint8_t>(addr & 0x7F), 0x00};
    uint8_t rx[2] = {0, 0};
    spi_.Transfer(tx, rx, 2);
    return rx[1];
}

void RF95Lora::WriteRegister(uint8_t addr, uint8_t value)
{
    uint8_t tx[2] = {static_cast<uint8_t>(addr | 0x80), value};
    uint8_t rx[2] = {0, 0};
    spi_.Transfer(tx, rx, 2);
}

void RF95Lora::ReadFifo(uint8_t* buf, uint8_t len)
{
    std::vector<uint8_t> tx(len + 1, 0x00);
    std::vector<uint8_t> rx(len + 1, 0x00);
    tx[0] = reg::kFifo & 0x7F;  // read
    spi_.Transfer(tx.data(), rx.data(), len + 1);
    std::memcpy(buf, rx.data() + 1, len);
}

void RF95Lora::WriteFifo(const uint8_t* buf, uint8_t len)
{
    std::vector<uint8_t> tx(len + 1);
    std::vector<uint8_t> rx(len + 1, 0);
    tx[0] = reg::kFifo | 0x80;  // write
    std::memcpy(tx.data() + 1, buf, len);
    spi_.Transfer(tx.data(), rx.data(), len + 1);
}

void RF95Lora::ReadSignalQuality(ReceivedPacket* pkt)
{
    // RSSI offset depends on frequency band (LF < 600 MHz).
    int rssi_offset = (config_.frequency_mhz < 600.0) ? -164 : -157;
    pkt->rssi       = rssi_offset + ReadRegister(reg::kPktRssiValue);

    auto snr_raw = static_cast<int8_t>(ReadRegister(reg::kPktSnrValue));
    pkt->snr     = snr_raw / 4.0f;

    // Adjust RSSI when SNR is negative.
    if (pkt->snr < 0.0f)
    {
        pkt->rssi += static_cast<int>(pkt->snr);
    }
}

}  // namespace lorawan