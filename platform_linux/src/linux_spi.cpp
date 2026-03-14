// linux_spi.cpp: Linux spidev HAL implementation.

#include "linux_spi.h"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

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

    ioctl(fd_, SPI_IOC_WR_MODE, &mode_);
    ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bits_);
    ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed_);

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
    tr.tx_buf        = (unsigned long)tx;
    tr.rx_buf        = (unsigned long)rx;
    tr.len           = len;
    tr.speed_hz      = speed_;
    tr.bits_per_word = bits_;

    return ioctl(fd_, SPI_IOC_MESSAGE(1), &tr) >= 0;
}

}  // namespace rpi_linux
}  // namespace lorawan