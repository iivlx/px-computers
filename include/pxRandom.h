#pragma once

#include <random>
#include <stdint>

uint8_t randbyte() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(0, 255);
  return dist(gen);
}

uint16_t randword() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(0, 65536);
  return dist(gen);
}
