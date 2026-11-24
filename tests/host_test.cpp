// SPDX-License-Identifier: BSD-3-Clause
// Host smoke test: the driver runs against VL53L4CXHal with no framework, and bus errors end calls instead of hanging.

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "vl53l4cx_class.h"

#define CHECK(cond)                                                       \
  do {                                                                    \
    if (!(cond)) {                                                        \
      std::fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
      std::exit(1);                                                       \
    }                                                                     \
  } while (0)

namespace {

class FakeHal : public VL53L4CXHal {
  public:
    bool write_registers(uint8_t address, uint16_t reg, const uint8_t *data, uint16_t len) override
    {
      last_address = address;
      if (nack) {
        return false;
      }
      for (uint16_t i = 0; i < len; i++) {
        regs[(uint16_t)(reg + i)] = data[i];
      }
      return true;
    }

    bool read_registers(uint8_t address, uint16_t reg, uint8_t *data, uint16_t len) override
    {
      last_address = address;
      reads++;
      if (nack) {
        return false;
      }
      for (uint16_t i = 0; i < len; i++) {
        data[i] = regs[(uint16_t)(reg + i)];
      }
      return true;
    }

    uint32_t millis() override
    {
      return now_ms;
    }
    void delay_ms(uint32_t ms) override
    {
      now_ms += ms;
    }
    void delay_us(uint32_t us) override
    {
      now_ms += (us + 999) / 1000;
    }
    std::vector<uint8_t> regs = std::vector<uint8_t>(0x10000, 0);
    uint32_t now_ms{0};
    uint32_t reads{0};
    uint8_t last_address{0};
    bool nack{false};
};

// The driver object is about 9.5 KB, too large for a stack frame on small targets
FakeHal hal;
VL53L4CX sensor(&hal);

}  // namespace

int main()
{
  // Model id 0xEB, module type 0xAA at register 0x010F, read big endian at the default 7-bit address 0x29
  hal.regs[VL53L4CX_IDENTIFICATION__MODEL_ID] = 0xEB;
  hal.regs[VL53L4CX_IDENTIFICATION__MODEL_ID + 1] = 0xAA;
  uint16_t id = 0;
  CHECK(sensor.VL53L4CX_GetSensorId(&id) == VL53L4CX_ERROR_NONE);
  CHECK(id == 0xEBAA);
  CHECK(hal.last_address == 0x29);

  // A new 8-bit address is written as its 7-bit form and used for the next transfers
  CHECK(sensor.VL53L4CX_SetDeviceAddress(0x58) == VL53L4CX_ERROR_NONE);
  CHECK(hal.regs[VL53L4CX_I2C_SLAVE__DEVICE_ADDRESS] == 0x2C);
  CHECK(sensor.VL53L4CX_GetSensorId(&id) == VL53L4CX_ERROR_NONE);
  CHECK(hal.last_address == 0x2C);

  // The boot wait polls until the firmware reports booted, then returns
  hal.regs[VL53L4CX_FIRMWARE__SYSTEM_STATUS] = 0x01;
  CHECK(sensor.VL53L4CX_WaitDeviceBooted() == VL53L4CX_ERROR_NONE);

  // Never booted: the wait times out on the HAL clock instead of spinning
  hal.regs[VL53L4CX_FIRMWARE__SYSTEM_STATUS] = 0x00;
  CHECK(sensor.VL53L4CX_WaitDeviceBooted() == VL53L4CX_ERROR_TIME_OUT);

  // A missing sensor (NACK) fails the call after one attempt; stm32duino looped forever here
  hal.nack = true;
  hal.reads = 0;
  CHECK(sensor.VL53L4CX_GetSensorId(&id) == VL53L4CX_ERROR_CONTROL_INTERFACE);
  CHECK(hal.reads == 1);
  uint8_t ready = 0;
  CHECK(sensor.VL53L4CX_GetMeasurementDataReady(&ready) == VL53L4CX_ERROR_CONTROL_INTERFACE);
  CHECK(sensor.VL53L4CX_SetDeviceAddress(0x52) == VL53L4CX_ERROR_CONTROL_INTERFACE);
  // A failed results read is reported, not turned into a measurement by the processing after it
  VL53L4CX_MultiRangingData_t data;
  CHECK(sensor.VL53L4CX_GetMultiRangingData(&data) == VL53L4CX_ERROR_CONTROL_INTERFACE);
  // A failed patch load stops the start instead of ranging with an incomplete patch
  CHECK(sensor.VL53L4CX_StartMeasurement() == VL53L4CX_ERROR_CONTROL_INTERFACE);

  std::puts("vl53l4cx host test passed");
  return 0;
}
