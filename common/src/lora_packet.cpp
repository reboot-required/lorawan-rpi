// lora_packet.cpp: packet serialization, deserialization, and CRC helpers.

#include "lora_packet.h"

#include <iomanip>
#include <sstream>

namespace lorawan
{

uint16_t CalculateCrc16(const uint8_t* data, size_t length)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i)
    {
        crc ^= static_cast<uint16_t>(data[i]) << 8;
        for (int bit = 0; bit < 8; ++bit)
        {
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
        }
    }
    return crc;
}

std::vector<uint8_t> LoraPacket::Serialize() const
{
    std::vector<uint8_t> buf;
    buf.reserve(4 + 2 + 1 + payload.size() + 2);

    // DevAddr (big-endian)
    buf.push_back(static_cast<uint8_t>((device_address >> 24) & 0xFF));
    buf.push_back(static_cast<uint8_t>((device_address >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((device_address >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>((device_address) & 0xFF));

    // FrameCounter (big-endian)
    buf.push_back(static_cast<uint8_t>((frame_counter >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>((frame_counter) & 0xFF));

    // Port
    buf.push_back(port);

    // Payload
    buf.insert(buf.end(), payload.begin(), payload.end());

    // CRC16 over everything so far
    uint16_t crc = CalculateCrc16(buf.data(), buf.size());
    buf.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>((crc) & 0xFF));

    return buf;
}

bool LoraPacket::Deserialize(const std::vector<uint8_t>& data, LoraPacket* packet)
{
    constexpr size_t kMinSize = 4 + 2 + 1 + 2;  // header + CRC, no payload
    if (data.size() < kMinSize || packet == nullptr)
    {
        return false;
    }

    const size_t body_len     = data.size() - 2;
    uint16_t     received_crc = (static_cast<uint16_t>(data[body_len]) << 8) | data[body_len + 1];
    uint16_t     computed_crc = CalculateCrc16(data.data(), body_len);
    if (received_crc != computed_crc)
    {
        return false;
    }

    packet->device_address =
        (static_cast<uint32_t>(data[0]) << 24) | (static_cast<uint32_t>(data[1]) << 16) |
        (static_cast<uint32_t>(data[2]) << 8) | (static_cast<uint32_t>(data[3]));

    packet->frame_counter = (static_cast<uint16_t>(data[4]) << 8) | data[5];

    packet->port = data[6];

    if (body_len > 7)
    {
        packet->payload.assign(data.begin() + 7,
                               data.begin() + static_cast<std::ptrdiff_t>(body_len));
    }
    else
    {
        packet->payload.clear();
    }
    return true;
}

std::string LoraPacket::ToString() const
{
    std::ostringstream oss;
    oss << "Packet{addr=0x" << std::hex << std::setfill('0') << std::setw(8) << device_address
        << ", cnt=" << std::dec << frame_counter << ", port=" << static_cast<int>(port)
        << ", payload=\"";

    for (uint8_t b : payload)
    {
        if (b >= 0x20 && b <= 0x7E)
        {
            oss << static_cast<char>(b);
        }
        else
        {
            oss << "\\x" << std::hex << std::setw(2) << static_cast<int>(b);
        }
    }
    oss << "\"(" << std::dec << payload.size() << "B)}";
    return oss.str();
}

}  // namespace lorawan