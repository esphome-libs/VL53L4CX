# VL53L4CX

Driver for the STMicroelectronics VL53L4CX time-of-flight sensor with multi-target detection, used by
[ESPHome](https://esphome.io). It is a fork of [stm32duino/VL53L4CX](https://github.com/stm32duino/VL53L4CX)
with the Arduino `Wire`, GPIO and timing calls replaced by a small platform interface, so the same code runs on
ESP-IDF, Arduino or any other framework.

## Changes from stm32duino/VL53L4CX

- `VL53L4CXHal` (`src/vl53l4cx_hal.h`) carries every platform call: register reads and writes over I2C, a
  millisecond clock and delays. `VL53L4CX` takes a `VL53L4CXHal *` instead of a `TwoWire *` and a pin.
- XSHUT is left to the caller, who can hold several sensors off while giving each its address. `VL53L4CX_On`/`Off`
  are gone, and `InitSensor()` expects a sensor fresh out of reset at the default address.
- A failed I2C transfer returns `VL53L4CX_ERROR_CONTROL_INTERFACE`. The Arduino version retried a read forever, so a
  missing or unpowered sensor hung the caller until a watchdog reset.
- `VL53L4CX_Dev_t` keeps only the fields the driver uses, and `VL53L4CX` has no virtual functions.
- `stdint.h` is used directly instead of guessing the platform's integer types.
- The ranging code, histogram processing and register settings are ST's, unchanged.
- The Arduino examples and Arduino IDE metadata are removed; `library.json` and `idf_component.yml` package it for
  PlatformIO and the ESP-IDF component registry.

## Usage

```cpp
#include "vl53l4cx_class.h"

class MyHal : public VL53L4CXHal {
 public:
  bool write_registers(uint8_t address, uint16_t reg, const uint8_t *data, uint16_t len) override;
  bool read_registers(uint8_t address, uint16_t reg, uint8_t *data, uint16_t len) override;
  uint32_t millis() override;
  void delay_ms(uint32_t ms) override;
  void delay_us(uint32_t us) override;
};

MyHal hal;
VL53L4CX sensor(&hal);  // about 9.5 KB; keep it out of the stack

// Cycle XSHUT low then high first, so the sensor is at its default address
sensor.InitSensor(0x52);  // 8-bit address; the HAL receives the 7-bit form (0x29)
sensor.VL53L4CX_SetDistanceMode(VL53L4CX_DISTANCEMODE_LONG);
sensor.VL53L4CX_StartMeasurement();
// When GPIO1 goes low, or VL53L4CX_GetMeasurementDataReady() reports ready:
VL53L4CX_MultiRangingData_t data;
sensor.VL53L4CX_GetMultiRangingData(&data);
sensor.VL53L4CX_ClearInterruptAndStartMeasurement();
```

Register addresses are 16 bits, sent most significant byte first, followed by the data bytes. Send each transfer to
the `address` argument: it changes after `VL53L4CX_SetDeviceAddress()`.

## Testing

`cmake -S . -B build && cmake --build build && ctest --test-dir build` builds the driver with no framework and runs a
smoke test against a fake platform interface.

## License

BSD-3-Clause, see [LICENSE.md](LICENSE.md). Most of ST's source files are dual licensed GPL-2.0+ or BSD-3-Clause; this
fork is distributed under BSD-3-Clause.
