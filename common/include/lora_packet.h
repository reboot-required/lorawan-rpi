/**
 * @file lora_packet.h
 * @brief LoRa packet serialization and CRC helpers.
 */

#ifndef LORAWAN_RPI_INCLUDE_LORA_PACKET_H_
#define LORAWAN_RPI_INCLUDE_LORA_PACKET_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace lorawan
{

/**
 * @brief LoRa packet in a simplified LoRaWAN-inspired format.
 *
 * Layout:
 * - DevAddr (4 bytes, big-endian)
 * - FrameCnt (2 bytes, big-endian)
 * - Port (1 byte)
 * - Payload (N bytes)
 * - CRC16-CCITT (2 bytes, big-endian)
 */
struct LoraPacket
{
    /** @brief Device address (big-endian when serialized). */
    uint32_t device_address = 0;

    /** @brief Frame counter (big-endian when serialized). */
    uint16_t frame_counter = 0;

    /** @brief Application port. */
    uint8_t port = 1;

    /** @brief Application payload bytes. */
    std::vector<uint8_t> payload;

    /**
     * @brief Serialize this packet including trailing CRC16.
     * @return Serialized packet bytes.
     */
    std::vector<uint8_t> Serialize() const;

    /**
     * @brief Parse packet bytes and validate trailing CRC16.
     * @param data Serialized packet bytes.
     * @param packet Output packet on success.
     * @return true when parsing and CRC check succeed.
     */
    static bool Deserialize(const std::vector<uint8_t>& data, LoraPacket* packet);

    /**
     * @brief Build a human-readable packet summary.
     * @return String representation.
     */
    std::string ToString() const;
};

/**
 * @brief Compute CRC-16/CCITT (poly 0x1021, init 0xFFFF).
 * @param data Input bytes.
 * @param length Number of input bytes.
 * @return CRC16 value.
 */
uint16_t CalculateCrc16(const uint8_t* data, size_t length);

}  // namespace lorawan

#endif  // LORAWAN_RPI_INCLUDE_LORA_PACKET_H_