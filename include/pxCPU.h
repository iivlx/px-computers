#pragma once

#include <utility>
#include <chrono>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <functional>

/* 
 * Stack mode is the first 2 bits, so 00, 01, 10, 11... 0, 64, 128, 192
 * Opcodes are the last 6 bits, so these are the same: 0-63, 64-127, 128-191, 192-255
 * 
 *
 *
 */

#include "pxMainboard.h"

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
  uint64_t getCycles() { return CYCLES; };

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



  void sleep(uint64_t n);
  std::function<void()> decode(uint8_t ir);
  void execute();
  void nop();
  uint8_t fetchByte();
  uint16_t fetchWord();

  uint8_t randbyte();
  uint16_t randword();

  //tmp
  void PxCPU::add();
  //tmp

  void pushStack(uint16_t value);
  uint16_t peekStack();
  uint16_t popStack();

  std::pair<uint16_t, uint16_t> resolveOperand(uint8_t mode);
  void mov();
  void store();
  void unaryOp(std::function<uint32_t(uint32_t)> op);
  void binaryOp(std::function<uint32_t(uint32_t, uint32_t)> op);
  void trinaryOp(std::function<uint32_t(uint32_t, uint32_t)> op);
  //void quaternaryOp(std::function<uint16_t(uint16_t, uint16_t)> op);
  void jmpOp(std::function<uint32_t(int32_t, int32_t)> operation);   // signed...
  void cmpOp();
  void setOp(uint16_t value);
  void push();
  void pop();
  void call();
  void ret();


  std::unordered_map<uint8_t, std::function<void()>> instructionSet;
  void initializeInstructionSet();

};
