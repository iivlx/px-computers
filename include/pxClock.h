#pragma once

#include "pxDevice.h"


class PxClock : public PxDevice {
public:


  PxClock(uint8_t frequency)
    : tickCount(0)
    , frequency(frequency)
    , running(true)
  {
  }

  uint8_t readByte(uint16_t address) const override {
    if (address == 0x0000) {
      return static_cast<uint8_t>(tickCount & 0xFF); // lower byte of tick count
    }
    else if (address == 0x0001) {
      return static_cast<uint8_t>((tickCount >> 8) & 0xFF); // upper byte of tick count
    }
    return 0;
  };

  void writeByte(uint16_t address, uint8_t value) override {
    if (address == 0x0001) { // set frequency
      frequency = value;
    }
    else if (address == 0x0002) { // start/stop
      running = (value != 0);
    }
  }

  void tick() {
    if (running) {
      tickCount++;
    }
  }

  void reset() {
    tickCount = 0;
  }

private:
  uint16_t tickCount;
  uint8_t frequency; // mHz
  bool running;


};
