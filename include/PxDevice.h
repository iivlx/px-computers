#pragma once

#include <cstdint>

class PxDevice {

public:

  PxDevice() = default;
  ~PxDevice() = default;

  virtual uint8_t readByte(uint16_t address) const = 0;
  virtual void writeByte(uint16_t address, uint8_t byte) = 0;

};
