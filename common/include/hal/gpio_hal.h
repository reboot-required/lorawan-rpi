/**
 * @file gpio_hal.h
 * @brief Platform-independent GPIO interface.
 */

#ifndef LORAWAN_RPI_INCLUDE_HAL_GPIO_HAL_H_
#define LORAWAN_RPI_INCLUDE_HAL_GPIO_HAL_H_

namespace lorawan
{
namespace hal
{

/**
 * @brief Abstract GPIO control interface.
 */
class GpioHal
{
   public:
    /** @brief Virtual destructor. */
    virtual ~GpioHal() = default;

    /**
     * @brief Initialize the GPIO line and direction.
     * @return true on success, false on failure.
     */
    virtual bool Init() = 0;

    /** @brief Drive GPIO high. */
    virtual void SetHigh() = 0;

    /** @brief Drive GPIO low. */
    virtual void SetLow() = 0;

    /**
     * @brief Read GPIO logic state.
     * @return 1 for high, 0 for low, negative value on error.
     */
    virtual int Read() = 0;
};

}  // namespace hal
}  // namespace lorawan

#endif  // LORAWAN_RPI_INCLUDE_HAL_GPIO_HAL_H_