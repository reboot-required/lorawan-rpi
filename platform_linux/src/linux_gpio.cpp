// linux_gpio.cpp: Linux libgpiod GPIO HAL implementation.

#include "linux_gpio.h"

#include "logger.h"

#include <cerrno>
#include <cstring>

namespace lorawan
{
namespace rpi_linux
{

LinuxGpio::LinuxGpio(int pin, bool output, const std::string& chip_name)
    : pin_(pin), output_(output), chip_name_(chip_name)
{
}

LinuxGpio::~LinuxGpio() { Release(); }

bool LinuxGpio::Init()
{
    if (initialized_)
    {
        return true;
    }

#ifdef LORAWAN_HAVE_LIBGPIOD
    chip_ = gpiod_chip_open_by_name(chip_name_.c_str());
    if (chip_ == nullptr)
    {
        Logger::Error("Failed to open GPIO chip " + chip_name_ + ": " + std::strerror(errno));
        return false;
    }

    line_ = gpiod_chip_get_line(chip_, pin_);
    if (line_ == nullptr)
    {
        Logger::Error("Failed to acquire GPIO line " + std::to_string(pin_) + ": " +
                      std::strerror(errno));
        Release();
        return false;
    }

    const int request_status = output_ ? gpiod_line_request_output(line_, "lorawan-rpi", 0)
                                       : gpiod_line_request_input(line_, "lorawan-rpi");
    if (request_status < 0)
    {
        Logger::Error("Failed to request GPIO line " + std::to_string(pin_) + ": " +
                      std::strerror(errno));
        Release();
        return false;
    }

    initialized_ = true;
    return true;
#else
    Logger::Error("GPIO backend unavailable: libgpiod was not found at build time");
    return false;
#endif
}

void LinuxGpio::SetHigh()
{
#ifdef LORAWAN_HAVE_LIBGPIOD
    if (!initialized_ || !output_)
    {
        return;
    }

    if (gpiod_line_set_value(line_, 1) < 0)
    {
        Logger::Error("Failed to drive GPIO line " + std::to_string(pin_) +
                      " high: " + std::strerror(errno));
    }
#else
    (void)pin_;
#endif
}

void LinuxGpio::SetLow()
{
#ifdef LORAWAN_HAVE_LIBGPIOD
    if (!initialized_ || !output_)
    {
        return;
    }

    if (gpiod_line_set_value(line_, 0) < 0)
    {
        Logger::Error("Failed to drive GPIO line " + std::to_string(pin_) +
                      " low: " + std::strerror(errno));
    }
#else
    (void)pin_;
#endif
}

int LinuxGpio::Read()
{
#ifdef LORAWAN_HAVE_LIBGPIOD
    if (!initialized_)
    {
        return -1;
    }

    const int value = gpiod_line_get_value(line_);
    if (value < 0)
    {
        Logger::Error("Failed to read GPIO line " + std::to_string(pin_) + ": " +
                      std::strerror(errno));
    }
    return value;
#else
    return -1;
#endif
}

void LinuxGpio::Release()
{
#ifdef LORAWAN_HAVE_LIBGPIOD
    if (line_ != nullptr)
    {
        gpiod_line_release(line_);
        line_ = nullptr;
    }

    if (chip_ != nullptr)
    {
        gpiod_chip_close(chip_);
        chip_ = nullptr;
    }

    initialized_ = false;
#endif
}

}  // namespace rpi_linux
}  // namespace lorawan