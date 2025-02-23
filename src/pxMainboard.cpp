#include <unordered_map>

#include "pxMainboard.h"
#include "pxCPU.h"


uint8_t PxMainboard::readByte(uint16_t address) const {
  auto [device, base_offset] = resolveDevice(address);

  if (device) {
    uint16_t device_address = address - base_offset;
    return device->readByte(device_address);
  }

  throw std::out_of_range("Invalid memory address");
}

uint16_t PxMainboard::readWord(uint16_t address) const {
  uint8_t high = readByte(address); // h
  uint8_t low = readByte(address + 1); // l
  uint16_t word = (high << 8) | (low & 0xFF);
  return word;
}

void PxMainboard::writeByte(uint16_t address, uint8_t value) {

  auto [device, base_offset] = resolveDevice(address);

  if (device) {
    uint16_t device_address = address - base_offset; // dev-local
    device->writeByte(device_address, value);
  }

  else {
    throw std::out_of_range("Invalid memory address");
  }

}

void PxMainboard::writeWord(uint16_t address, uint16_t value) {
  uint8_t h = value >> 8;
  uint8_t l = value & 0xFF;
  writeByte(address, h); // h
  writeByte(address+1, l); // l
}


void PxMainboard::tick() {
  clock->tick();
  for (auto it = cpus.begin(); it != cpus.end(); it++) {
    it->second->tick();
  }
};

std::pair<PxDevice*, uint16_t> PxMainboard::resolveDevice(uint16_t address) const {
  for (const auto& [device, range] : devices) {
    uint16_t start_address = range.first;
    uint16_t size = range.second;

    if (address >= start_address && address < start_address + size) {
      return { device, start_address };
    }
  }
  return { nullptr, 0 };
}


void PxMainboard::addDevice(std::string name, PxDevice* device, uint16_t start_address, uint16_t size) {
  devices[device] = std::make_pair(start_address, size);

};

void PxMainboard::addCPU(PxCPU* cpu) {
  cpus.emplace(0, cpu);
};
