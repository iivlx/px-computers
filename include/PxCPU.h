#pragma once

#include <utility>
#include <chrono>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <functional>

#include "PxMainboard.h"

/* stack modes */

#define S_NONE  0b000    //  .
#define S_OUT   0b001    //  >
#define S_IN    0b010    //  <
#define S_CHK   0b011    //  <>
#define S_L_OUT 0b100    //  ^
#define S_L_CHK 0b110    //  <^

/* addressing modes */

#define IMM16 0b000
#define IMM8  0b001
#define DIR16 0b010
#define DIR8  0b011
#define IND16 0b100
#define IND8  0b101

#define IMMEDIATE16 IMM16
#define IMMEDIATE8 IMM8
#define DIRECT16 DIR16
#define DIRECT8 DIR8
#define INDIRECT16 IND16
#define INDIRECT8 IND8

 // Big-endian...

using pxbyte = uint8_t;
using pxword = uint16_t;
using pxdword = uint32_t;
using pxqword = uint64_t;

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

  std::function<void()> decode(pxbyte ir);
  std::pair<pxword, pxword> resolveOperand(pxbyte mode, pxword word);
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
