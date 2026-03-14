/**
 * @file linux_delay.h
 * @brief Linux implementation of DelayHal.
 */

#ifndef LORAWAN_PLATFORM_LINUX_LINUX_DELAY_H_
#define LORAWAN_PLATFORM_LINUX_LINUX_DELAY_H_

#include "hal/delay_hal.h"

namespace lorawan
{
namespace rpi_linux
{

/**
 * @brief Linux delay/tick implementation.
 */
class LinuxDelay : public hal::DelayHal
{
   public:
    /**
     * @brief Sleep the current thread for a number of milliseconds.
     * @param ms Delay duration.
     */
    void DelayMs(uint32_t ms) override;

    /**
     * @brief Read monotonic tick count in milliseconds.
     * @return Tick value in milliseconds.
     */
    uint32_t GetTickMs() override;
};

}  // namespace rpi_linux
}  // namespace lorawan

#endif  // LORAWAN_PLATFORM_LINUX_LINUX_DELAY_H_