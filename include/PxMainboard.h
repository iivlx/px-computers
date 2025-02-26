#pragma once

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "PxCPU.h" 
#include "PxClock.h" 
#include "PxDevice.h" 
#include "PxDisplay.h"

class PxCPU;
//class PxDisplay;

class PxMainboard : PxDevice {
public:
  std::unordered_map<int, PxCPU*> cpus; // attached cpus
  std::unordered_map<PxDevice*, std::pair<uint16_t, uint16_t>> devices; // devices mapped to memory
  PxClock* clock = nullptr;
  PxDisplay* main_display = nullptr;
  uint16_t reset_vector;
  std::vector<uint8_t> irq_buffer;

  PxMainboard() {};
  ~PxMainboard() {};

  void tick();

  void resetButton(); // press the reset button
  void powerButton(); // press the power button

  uint8_t readByte(uint16_t address) const;
  uint16_t readWord(uint16_t address) const;
  void writeByte(uint16_t address, uint8_t byte);
  void writeWord(uint16_t address, uint16_t byte);

  void addDevice(std::string name, PxDevice* device, uint16_t start_address, uint16_t size);
  void addCPU(PxCPU* cpu);

  PxDevice* getDeviceAtAddress(uint16_t address) const;

  void setClock(PxClock* c) { clock = c; };
  void setResetVector(uint16_t address) { reset_vector = address; };
  uint16_t getResetVector() const { return reset_vector; };

  std::pair<PxDevice*, uint16_t> PxMainboard::resolveDevice(uint16_t address) const;

};
