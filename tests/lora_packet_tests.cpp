#include "lora_packet.h"

#include <cstdint>
#include <iostream>
#include <vector>

namespace
{

bool TestSerializeDeserializeRoundTrip()
{
    lorawan::LoraPacket pkt;
    pkt.device_address = 0x01020304;
    pkt.frame_counter  = 42;
    pkt.port           = 7;
    pkt.payload        = {'H', 'e', 'l', 'l', 'o'};

    const std::vector<uint8_t> encoded = pkt.Serialize();

    lorawan::LoraPacket decoded;
    if (!lorawan::LoraPacket::Deserialize(encoded, &decoded))
    {
        std::cerr << "Deserialize failed for valid packet" << std::endl;
        return false;
    }

    return decoded.device_address == pkt.device_address &&
           decoded.frame_counter == pkt.frame_counter && decoded.port == pkt.port &&
           decoded.payload == pkt.payload;
}

bool TestDeserializeRejectsCrcMismatch()
{
    lorawan::LoraPacket pkt;
    pkt.device_address = 0x0A0B0C0D;
    pkt.frame_counter  = 1;
    pkt.port           = 1;
    pkt.payload        = {0x10, 0x20, 0x30};

    std::vector<uint8_t> encoded = pkt.Serialize();
    encoded[3] ^= 0x01;

    lorawan::LoraPacket decoded;
    if (lorawan::LoraPacket::Deserialize(encoded, &decoded))
    {
        std::cerr << "CRC mismatch was not detected" << std::endl;
        return false;
    }

    return true;
}

bool TestKnownCrcVector()
{
    static const uint8_t kData[]      = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    constexpr uint16_t   kExpectedCrc = 0x29B1;

    const uint16_t crc = lorawan::CalculateCrc16(kData, sizeof(kData));
    if (crc != kExpectedCrc)
    {
        std::cerr << "Unexpected CRC value: got " << std::hex << crc << " expected " << kExpectedCrc
                  << std::dec << std::endl;
        return false;
    }

    return true;
}

}  // namespace

int main()
{
    const bool ok_round_trip = TestSerializeDeserializeRoundTrip();
    const bool ok_crc_reject = TestDeserializeRejectsCrcMismatch();
    const bool ok_crc_vector = TestKnownCrcVector();

    if (!(ok_round_trip && ok_crc_reject && ok_crc_vector))
    {
        return 1;
    }

    std::cout << "All lora_packet tests passed" << std::endl;
    return 0;
}
