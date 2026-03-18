/**
 * @file linux_gpio.h
 * @brief Linux libgpiod GPIO implementation.
 */

#ifndef LORAWAN_PLATFORM_LINUX_LINUX_GPIO_H_
#define LORAWAN_PLATFORM_LINUX_LINUX_GPIO_H_

#include "hal/gpio_hal.h"

#ifdef LORAWAN_HAVE_LIBGPIOD
#include <gpiod.h>
#else
struct gpiod_chip;
struct gpiod_line;
#endif

#include <string>

namespace lorawan
{
namespace rpi_linux
{

/**
 * @brief GPIO implementation using Linux libgpiod line requests.
 */
class LinuxGpio : public hal::GpioHal
{
   public:
    /**
     * @brief Construct a GPIO wrapper.
     * @param pin BCM pin number.
     * @param output true for output mode, false for input mode.
     * @param chip_name GPIO chip name, defaults to gpiochip0.
     */
    LinuxGpio(int pin, bool output, const std::string& chip_name = "gpiochip0");

    /** @brief Destructor. Releases the requested GPIO line if held. */
    ~LinuxGpio() override;

    /** @copydoc hal::GpioHal::Init */
    bool Init() override;

    /** @copydoc hal::GpioHal::SetHigh */
    void SetHigh() override;

    /** @copydoc hal::GpioHal::SetLow */
    void SetLow() override;

    /** @copydoc hal::GpioHal::Read */
    int Read() override;

   private:
    void Release();

    int         pin_;
    bool        output_;
    std::string chip_name_;
    gpiod_chip* chip_        = nullptr;
    gpiod_line* line_        = nullptr;
    bool        initialized_ = false;
};

}  // namespace rpi_linux
}  // namespace lorawan

#endif  // LORAWAN_PLATFORM_LINUX_LINUX_GPIO_H_