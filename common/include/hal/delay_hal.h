/**
 * @file delay_hal.h
 * @brief Platform-independent delay/timing interface.
 */

#ifndef LORAWAN_RPI_INCLUDE_HAL_DELAY_HAL_H_
#define LORAWAN_RPI_INCLUDE_HAL_DELAY_HAL_H_

#include <cstdint>

namespace lorawan
{
namespace hal
{

/**
 * @brief Abstract interface for delays and monotonic tick access.
 */
class DelayHal
{
   public:
    /** @brief Virtual destructor. */
    virtual ~DelayHal() = default;

    /**
     * @brief Block execution for a number of milliseconds.
     * @param ms Delay duration in milliseconds.
     */
    virtual void DelayMs(uint32_t ms) = 0;

    /**
     * @brief Read a monotonic tick counter in milliseconds.
     * @return Tick value in milliseconds.
     */
    virtual uint32_t GetTickMs() = 0;
};

}  // namespace hal
}  // namespace lorawan

#endif  // LORAWAN_RPI_INCLUDE_HAL_DELAY_HAL_H_