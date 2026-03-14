// linux_delay.cpp: Linux delay/tick HAL implementation.

#include "linux_delay.h"

#include <chrono>
#include <thread>

namespace lorawan
{
namespace rpi_linux
{

void LinuxDelay::DelayMs(uint32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

uint32_t LinuxDelay::GetTickMs()
{
    using namespace std::chrono;

    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace rpi_linux
}  // namespace lorawan