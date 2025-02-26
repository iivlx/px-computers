#pragma once

class PxGPU : public PxDevice {


  PxGPU() {};
  ~PxGPU() = default;

  uint8_t readByte(uint16_t address) const {};
  void writeByte(uint16_t address, uint8_t byte) {};

};

