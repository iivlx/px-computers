#pragma once


class PxROM : public PxDevice {
public:

  PxROM(uint16_t size) {
    for (int i = 0; i < size; ++i) {
      ROM.emplace_back(0x00);
    }
  };
  ~PxROM() = default;


  std::vector<uint8_t> ROM;


  uint8_t readByte(uint16_t address) const {
    return ROM[address];
  };

  void writeByte(uint16_t address, uint8_t byte) {
    ROM[address] = byte;
  };
  void writeBytes(uint16_t address, uint8_t* bytes, size_t size) {
    for (int i = 0; i < size; i++) {
      ROM[address + i] = *(bytes + i);
    }
  };

};
