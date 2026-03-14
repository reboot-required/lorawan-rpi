/**
 * @file linux_gpio.h
 * @brief Linux sysfs GPIO implementation.
 */

#ifndef LORAWAN_PLATFORM_LINUX_LINUX_GPIO_H_
#define LORAWAN_PLATFORM_LINUX_LINUX_GPIO_H_

#include "hal/gpio_hal.h"

#include <string>

namespace lorawan
{
namespace rpi_linux
{

/**
 * @brief GPIO implementation using Linux sysfs GPIO files.
 */
class LinuxGpio : public hal::GpioHal
{
   public:
    /**
     * @brief Construct a GPIO wrapper.
     * @param pin BCM pin number.
     * @param output true for output mode, false for input mode.
     */
    LinuxGpio(int pin, bool output);

    /** @copydoc hal::GpioHal::Init */
    bool Init() override;

    /** @copydoc hal::GpioHal::SetHigh */
    void SetHigh() override;

    /** @copydoc hal::GpioHal::SetLow */
    void SetLow() override;

    /** @copydoc hal::GpioHal::Read */
    int Read() override;

   private:
    /**
     * @brief Write a value string to a sysfs file.
     * @param path Absolute sysfs file path.
     * @param value Value to write.
     * @return true on success, false on failure.
     */
    bool WriteFile(const std::string& path, const std::string& value);

    /**
     * @brief Read a value string from a sysfs file.
     * @param path Absolute sysfs file path.
     * @return File content string or empty on error.
     */
    std::string ReadFile(const std::string& path);

    int         pin_;
    bool        output_;
    std::string base_;
};

}  // namespace rpi_linux
}  // namespace lorawan

#endif  // LORAWAN_PLATFORM_LINUX_LINUX_GPIO_H_