/**
 * @file spi_hal.h
 * @brief Platform-independent SPI interface.
 */

#ifndef LORAWAN_RPI_INCLUDE_HAL_SPI_HAL_H_
#define LORAWAN_RPI_INCLUDE_HAL_SPI_HAL_H_

#include <cstddef>
#include <cstdint>

namespace lorawan
{
namespace hal
{

/**
 * @brief Abstract SPI bus interface used by the radio driver.
 */
class SpiHal
{
   public:
    /** @brief Virtual destructor. */
    virtual ~SpiHal() = default;

    /**
     * @brief Open and configure the SPI device.
     * @return true on success, false on failure.
     */
    virtual bool Open() = 0;

    /** @brief Close the SPI device if open. */
    virtual void Close() = 0;

    /**
     * @brief Transfer data over SPI in full-duplex mode.
     * @param tx Pointer to TX bytes.
     * @param rx Pointer to RX buffer.
     * @param len Number of bytes to transfer.
     * @return true on success, false on failure.
     */
    virtual bool Transfer(const uint8_t* tx, uint8_t* rx, size_t len) = 0;
};

}  // namespace hal
}  // namespace lorawan

#endif  // LORAWAN_RPI_INCLUDE_HAL_SPI_HAL_H_
