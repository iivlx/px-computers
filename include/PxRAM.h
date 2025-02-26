#pragma once


class PxRAM : public PxDevice {
public:

  PxRAM(uint16_t size, uint8_t fill=0x00) {
    for (int i = 0; i < size; ++i) {
      RAM.emplace_back(fill);
    }
  };
  ~PxRAM() = default;

  std::vector<uint8_t> RAM;

  uint8_t readByte(uint16_t address) const {
    return RAM[address];
  };

  void writeByte(uint16_t address, uint8_t byte) {
    RAM[address] = byte;
  };

};
