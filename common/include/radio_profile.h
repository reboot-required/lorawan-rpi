/**
 * @file radio_profile.h
 * @brief Shared radio defaults and policy used by node and gateway apps.
 */

#ifndef LORAWAN_RPI_INCLUDE_RADIO_PROFILE_H_
#define LORAWAN_RPI_INCLUDE_RADIO_PROFILE_H_

#include <cmath>

namespace lorawan
{

namespace radio_profile
{

constexpr double kOnlyFrequencyMhz       = 868.1;
constexpr int    kDefaultBandwidthKhz    = 125;
constexpr int    kDefaultCodingRate      = 5;
constexpr int    kDefaultSyncWord        = 0x34;
constexpr bool   kDefaultCrcEnabled      = true;
constexpr int    kNodeDefaultPowerDbm    = 17;
constexpr int    kGatewayDefaultPowerDbm = 14;

inline bool IsAllowedFrequency(double frequency_mhz)
{
    return std::fabs(frequency_mhz - kOnlyFrequencyMhz) <= 0.0001;
}

}  // namespace radio_profile

}  // namespace lorawan

#endif  // LORAWAN_RPI_INCLUDE_RADIO_PROFILE_H_