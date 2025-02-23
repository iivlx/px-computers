#pragma once

#include "pxDevice.h"

enum PIXELDISPLAY {
  // write addresses
  SKIP = 0,
  CLEAR = 1,
  // read addresses
  READ = 0,
};


class PxKeyboard : public PxDevice {
public:


  PxKeyboard() {};
  ~PxKeyboard() {};

  uint8_t readByte(uint16_t address) const;
  void writeByte(uint16_t address, uint8_t value);

  std::vector<uint8_t>* charBuffer;



};