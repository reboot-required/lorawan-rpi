// linux_gpio.cpp: Linux sysfs GPIO HAL implementation.

#include "linux_gpio.h"

#include "logger.h"

#include <sys/stat.h>
#include <chrono>
#include <fstream>
#include <thread>

namespace lorawan
{
namespace rpi_linux
{

namespace
{

bool PathExists(const std::string& path)
{
    struct stat st{};
    return stat(path.c_str(), &st) == 0;
}

}  // namespace

LinuxGpio::LinuxGpio(int pin, bool output)
    : pin_(pin), output_(output), base_("/sys/class/gpio/gpio" + std::to_string(pin))
{
}

bool LinuxGpio::Init()
{
    if (!PathExists(base_) && !WriteFile("/sys/class/gpio/export", std::to_string(pin_)))
    {
        Logger::Error("Failed to export GPIO pin " + std::to_string(pin_));
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (!WriteFile(base_ + "/direction", output_ ? "out" : "in"))
    {
        Logger::Error("Failed to set GPIO direction for pin " + std::to_string(pin_));
        return false;
    }

    return true;
}

void LinuxGpio::SetHigh()
{
    if (output_) WriteFile(base_ + "/value", "1");
}

void LinuxGpio::SetLow()
{
    if (output_) WriteFile(base_ + "/value", "0");
}

int LinuxGpio::Read()
{
    std::string v = ReadFile(base_ + "/value");
    if (v.empty()) return -1;
    return v[0] == '1';
}

bool LinuxGpio::WriteFile(const std::string& path, const std::string& value)
{
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << value;
    return static_cast<bool>(f);
}

std::string LinuxGpio::ReadFile(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::string v;
    std::getline(f, v);
    return v;
}

}  // namespace rpi_linux
}  // namespace lorawan