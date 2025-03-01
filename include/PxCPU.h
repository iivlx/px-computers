#pragma once

#include <utility>
#include <chrono>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <functional>

#include "PxMainboard.h"

 // Big-endian...

class PxMainboard;

class PxCPU {
public:
  PxCPU(PxMainboard* mb)
    : mainboard(mb)
    , CYCLES(0)
    , cycleCost(0)
  {
    initializeInstructionSet();
    reset();
  };

  ~PxCPU() {};

  void tick();
  void halt();
  void reset();

  uint64_t getCycles();
  int getRunningTime();

  PxMainboard* mainboard;

private:
  /* CPU INTERNALS */ // kinda needs some work here.....
  uint64_t CYCLES;
  uint8_t IR;
  uint16_t PC;
  uint16_t SP;
  uint16_t IVT;
  bool INTERRUPT;
  bool ZERO_PAGE;
  uint64_t cycleCost;
  std::chrono::steady_clock::time_point startupTime;
  /* ------------- */

  uint8_t fetchByte();
  uint16_t fetchWord();

  void pushStack(uint16_t value);
  uint16_t peekStack();
  uint16_t popStack();

  std::function<void()> decode(uint8_t ir);
  std::pair<uint16_t, uint16_t> resolveOperand(uint8_t mode);
  void mov();
  void store();
  void unaryOp(std::function<uint32_t(uint32_t)> op);
  void binaryOp(std::function<uint32_t(uint32_t, uint32_t)> op);
  void trinaryOp(std::function<uint32_t(uint32_t, uint32_t)> op);
  void jmpOp(std::function<uint32_t(int32_t, int32_t)> operation);   // signed...
  void cmpOp();
  void setOp(uint16_t value);
  void push();
  void pop();
  void call();
  void ret();
  void nop();

  std::unordered_map<uint8_t, std::function<void()>> instructionSet;
  void initializeInstructionSet();

};
