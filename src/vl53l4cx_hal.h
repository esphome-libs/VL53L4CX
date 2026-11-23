// SPDX-License-Identifier: BSD-3-Clause
/*
 * Platform interface of the VL53L4CX driver: the bus, the clock and the XSHUT pin.
 * Implement it for your platform and pass it to the VL53L4CX constructor.
 */

#ifndef _VL53L4CX_HAL_H_
#define _VL53L4CX_HAL_H_

#include <cstdint>

class VL53L4CXHal {
  public:
    virtual ~VL53L4CXHal() = default;

    /**
     * Write `len` bytes starting at the 16-bit register `reg` of the device at the 7-bit I2C `address`.
     * @return 0 on success, non-zero on a bus error or NACK.
     */
    virtual int write_registers(uint8_t address, uint16_t reg, const uint8_t *data, uint16_t len) = 0;

    /**
     * Read `len` bytes starting at the 16-bit register `reg` of the device at the 7-bit I2C `address`.
     * @return 0 on success, non-zero on a bus error or NACK. Never retries forever.
     */
    virtual int read_registers(uint8_t address, uint16_t reg, uint8_t *data, uint16_t len) = 0;

    /** Milliseconds since an arbitrary start; may wrap. */
    virtual uint32_t millis() = 0;

    virtual void delay_ms(uint32_t ms) = 0;
    virtual void delay_us(uint32_t us) = 0;

    /** Drive XSHUT high (true, sensor on) or low (false, sensor off). Does nothing when no pin is wired. */
    virtual void set_xshut(bool high)
    {
      (void)high;
    }
};

#endif /* _VL53L4CX_HAL_H_ */
