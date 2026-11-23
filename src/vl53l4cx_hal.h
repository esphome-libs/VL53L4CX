// SPDX-License-Identifier: BSD-3-Clause
/*
 * Platform interface of the VL53L4CX driver: register access over I2C, a clock and delays.
 * Implement it for your platform and pass it to the VL53L4CX constructor. XSHUT is left to the caller.
 */

#ifndef _VL53L4CX_HAL_H_
#define _VL53L4CX_HAL_H_

#include <cstdint>

class VL53L4CXHal {
  public:
    /**
     * Write `len` bytes starting at the 16-bit register `reg` (sent most significant byte first) of the device at
     * the 7-bit I2C `address`. Use `address`, not a stored one: it changes after VL53L4CX_SetDeviceAddress().
     * @return true on success, false on a bus error or NACK.
     */
    virtual bool write_registers(uint8_t address, uint16_t reg, const uint8_t *data, uint16_t len) = 0;

    /**
     * Read `len` bytes starting at the 16-bit register `reg` of the device at the 7-bit I2C `address`, in one
     * write-then-read transaction. Must not retry forever.
     * @return true on success, false on a bus error or NACK.
     */
    virtual bool read_registers(uint8_t address, uint16_t reg, uint8_t *data, uint16_t len) = 0;

    /** Milliseconds since an arbitrary start; may wrap. */
    virtual uint32_t millis() = 0;

    virtual void delay_ms(uint32_t ms) = 0;
    virtual void delay_us(uint32_t us) = 0;

  protected:
    ~VL53L4CXHal() = default;
};

#endif /* _VL53L4CX_HAL_H_ */
