#pragma once

#include <vector>

#include "PxDevice.h"

//enum PXKEYBOARD {
//  // write addresses
//  KEY = 0,
//  ERASE = 1,
//  CLEAR = 2,
//
//  // read addresses
//  READ = 0,
//};

class PxKeyboard : public PxDevice {
public:


  PxKeyboard(int buffer_size);
  ~PxKeyboard();

  uint8_t readByte(uint16_t address) const;
  void writeByte(uint16_t address, uint8_t value);

  int buffer_size;
  std::vector<uint8_t>* charBuffer;

};
