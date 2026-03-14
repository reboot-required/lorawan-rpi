/**
 * @file linux_spi.h
 * @brief Linux spidev implementation of SpiHal.
 */

#ifndef LORAWAN_PLATFORM_LINUX_LINUX_SPI_H_
#define LORAWAN_PLATFORM_LINUX_LINUX_SPI_H_

#include "hal/spi_hal.h"

#include <string>

namespace lorawan
{
namespace rpi_linux
{

/**
 * @brief SPI implementation using Linux spidev.
 */
class LinuxSpi : public hal::SpiHal
{
   public:
    /**
     * @brief Construct an SPI wrapper for a spidev device node.
     * @param device Device path, for example `/dev/spidev0.0`.
     */
    explicit LinuxSpi(const std::string& device);

    /** @brief Destructor. Closes the SPI file descriptor if open. */
    ~LinuxSpi() override;

    /** @copydoc hal::SpiHal::Open */
    bool Open() override;

    /** @copydoc hal::SpiHal::Close */
    void Close() override;

    /** @copydoc hal::SpiHal::Transfer */
    bool Transfer(const uint8_t* tx, uint8_t* rx, size_t len) override;

   private:
    std::string device_;
    int         fd_ = -1;

    uint32_t speed_ = 500000;
    uint8_t  mode_  = 0;
    uint8_t  bits_  = 8;
};

}  // namespace rpi_linux
}  // namespace lorawan

#endif  // LORAWAN_PLATFORM_LINUX_LINUX_SPI_H_