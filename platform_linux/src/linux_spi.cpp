// linux_spi.cpp: Linux spidev HAL implementation.

#include "linux_spi.h"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>

namespace lorawan
{
namespace rpi_linux
{

LinuxSpi::LinuxSpi(const std::string& device) : device_(device) {}

LinuxSpi::~LinuxSpi() { Close(); }

bool LinuxSpi::Open()
{
    fd_ = open(device_.c_str(), O_RDWR);
    if (fd_ < 0) return false;

    if (ioctl(fd_, SPI_IOC_WR_MODE, &mode_) < 0)
    {
        Close();
        return false;
    }
    if (ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bits_) < 0)
    {
        Close();
        return false;
    }
    if (ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed_) < 0)
    {
        Close();
        return false;
    }

    return true;
}

void LinuxSpi::Close()
{
    if (fd_ >= 0)
    {
        close(fd_);
        fd_ = -1;
    }
}

bool LinuxSpi::Transfer(const uint8_t* tx, uint8_t* rx, size_t len)
{
    if (fd_ < 0) return false;

    spi_ioc_transfer tr{};
    tr.tx_buf        = static_cast<__u64>(reinterpret_cast<std::uintptr_t>(tx));
    tr.rx_buf        = static_cast<__u64>(reinterpret_cast<std::uintptr_t>(rx));
    tr.len           = len;
    tr.speed_hz      = speed_;
    tr.bits_per_word = bits_;

    return ioctl(fd_, SPI_IOC_MESSAGE(1), &tr) >= 0;
}

}  // namespace rpi_linux
}  // namespace lorawan