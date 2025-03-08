#include <chrono>
#include <thread>
#include <math.h>

#include "PxCPU.h"
#include "PxMainboard.h"
#include "PxDisplay.h"

#include "PxDevice.h"

#include "pxMath.h"
#include "pxFloat.h"

/* cpu cycle */

void PxCPU::tick() {
  if (cycleCost > 0) { --cycleCost; return; } // executing...(however we execute THEN wait on execution... somehow lol)
  CYCLES++;

  IR = fetchByte();
  auto instruction = decode(IR);
  instruction();
}

/* cpu reset */

void PxCPU::reset() {
  //PC = mainboard->getResetVector(); // doesn't work // really lol...// yes lol.. you should fix it..
  PC = 0xFE20; // ...
  SP = 0xFFFF;
  IR = 0;
  CYCLES = 0;
  INTERRUPT = 0;
  IVT = 0;
  cycleCost = 0;

  startupTime = std::chrono::high_resolution_clock::now();
}

/* timing functions */

uint64_t PxCPU::getCycles() { return CYCLES; };

int PxCPU::getRunningTime() {
  auto now = std::chrono::high_resolution_clock::now();
  auto time_span = std::chrono::duration_cast<std::chrono::milliseconds>(now - startupTime);
  return time_span.count();
}

/* fetching functions */

pxbyte PxCPU::fetchByte() {
  return mainboard->readByte(PC++);
};

pxword PxCPU::fetchWord() {
  // there is a lot of optimization...
  pxword word = mainboard->readByte(PC++) << 8;
  word |= mainboard->readByte(PC++);
  return word;
};

/* operand functions */

std::pair<pxword, pxword> PxCPU::resolveOperand(pxbyte mode, pxword number) {
  pxword address = 0;
  pxword value = 0;

  switch (mode) {
    case IMM16:
      value = number;
      break;
    case IMM8:
      value = number & 0xFF;
      break;

    case DIR16:
      address = number;
      value = mainboard->readWord(address);
      break;
    case DIR8:
      address = number;
      value = mainboard->readByte(address);
      break;

    case IND16:
      address = mainboard->readWord(number);
      value = mainboard->readWord(address);
      break;
    case IND8:
      address = mainboard->readWord(number);
      value = mainboard->readByte(address);
      break;

    default:
      throw std::runtime_error("Invalid operand mode");
  }

  return { address, value };
}

/* stack operations */

void PxCPU::pushStack(uint16_t value) {
  SP -= 2;
  mainboard->writeByte(SP, value >> 8);         // h
  mainboard->writeByte(SP + 1, value & 0xFF);   // l
}

uint16_t PxCPU::peekStack(){
  return mainboard->readWord(SP);
}

uint16_t PxCPU::popStack() {
  uint16_t value = mainboard->readWord(SP);
  SP += 2;
  return value;
}

/* cpu operations */

void PxCPU::nop() {
  // should be able to still do stack nops... >, <, <>
  // > : take off stack and discard
  // <> : really do nothing... I guess break if stack is empty?
  // < : put 0 on stack?
}

void PxCPU::binaryOp(std::function<uint32_t(uint32_t, uint32_t)> operation) {
  // modes
  uint8_t stack_mode = fetchByte();
  uint8_t mode = fetchByte();

  bool left_signed = (mode & 0b10000000) != 0;             // bit 7
  uint8_t left_mode = (mode >> 4) & 0b0111;                // bits 6-4
  bool right_signed = (mode & 0b00001000) != 0;            // bit 3
  uint8_t right_mode = mode & 0b0111;                      // bits 2-0

  // operands
  auto [left_a, left_v] = resolveOperand(left_mode, fetchWord());
  auto [right_a, right_v] = resolveOperand(right_mode, fetchWord());

  // interpret sign
  uint32_t left_value = left_signed ? static_cast<int16_t>(left_v) : static_cast<uint16_t>(left_v);
  uint32_t right_value = right_signed ? static_cast<int16_t>(right_v) : static_cast<uint16_t>(right_v);

  // pop from stack
  uint16_t carry_in = 0;
  if (stack_mode == S_OUT || stack_mode == S_CHK) { // '>' or '<>'
    carry_in = popStack();
  }

  // operation
  uint32_t result = operation(left_value, right_value);
  result += carry_in;
  uint16_t low = result & 0xFFFF;
  uint16_t high = (result >> 0x10);

  // write
  if (left_mode == IND16 || left_mode == DIR16) {    // word
    mainboard->writeWord(left_a, low);
  }
  else if (left_mode == IND8 || left_mode == DIR8) { // byte
    mainboard->writeByte(left_a, static_cast<uint8_t>(low));
  }

  // push to stack
  if (stack_mode == S_IN || stack_mode == S_CHK) { // '<' or '<>'
    pushStack(high);
  }
}

void PxCPU::unaryOp(std::function<uint32_t(uint32_t)> operation) {
  // mode
  uint8_t stack_mode = fetchByte();
  uint8_t mode = fetchByte();
  uint8_t left_mode = (mode >> 4) & 0b0111;

  // operands
  auto [left_a, left_v] = resolveOperand(left_mode, fetchWord());

  // pop from stack
  int16_t carry_in = 0;
  if (stack_mode == S_OUT || stack_mode == S_CHK) {
    carry_in = static_cast<int16_t>(popStack());
  }

  // operation
  uint32_t result = operation(left_v);
  result += carry_in;
  uint16_t low = result & 0xFFFF;
  uint16_t high = (result >> 0x10);

  // write
  if (left_mode == IND16 || left_mode == DIR16) {    // word
    mainboard->writeWord(left_a, low);
  }
  else if (left_mode == IND8 || left_mode == DIR8) { // byte
    mainboard->writeByte(left_a, static_cast<uint8_t>(low));
  }

  // push to stack
  if (stack_mode == S_IN || stack_mode == S_CHK) {
    pushStack(high);
  }
}

void PxCPU::jmpOp(std::function<uint32_t(int32_t, int32_t)> operation) {
  // mode
  uint8_t stack_mode = fetchByte();
  uint8_t mode = fetchByte();
  uint8_t left_mode = (mode >> 4 ) & 0b0111;              // bits 6-4

  // operands
  auto [left_a, left_v] = resolveOperand(left_mode, fetchWord());

  // pop from stack
  int16_t carry_in = 0;
  if (stack_mode == S_OUT || stack_mode == S_CHK) { // '>' or '<>'
    carry_in = static_cast<int16_t>(popStack());  // signed
  }

  // comparison
  uint32_t result = operation(carry_in, 0);

  // jump
  if (result == true) {
    PC = left_v;
  }

  // push to stack
  if (stack_mode == S_IN || stack_mode == S_CHK) { // '<' or '<>'
    pushStack(carry_in);
  }
}

void PxCPU::cmpOp() {
  // modes
  uint8_t stack_mode = fetchByte();
  uint8_t mode = fetchByte();
  bool left_signed = (mode & 0b10000000) != 0;             //bit 7
  uint8_t left_mode = (mode >> 4) & 0b0111;                //bits 6-4
  bool right_signed = (mode & 0b00001000) != 0;            //bit 3 
  uint8_t right_mode = mode & 0b0111;                      //bits 2-0

  // operands
  auto [left_a, left_v] = resolveOperand(left_mode, fetchWord());
  auto [right_a, right_v] = resolveOperand(right_mode, fetchWord());

  // interpret sign
  uint32_t left_value = left_signed ? static_cast<int16_t>(left_v) : static_cast<uint16_t>(left_v);
  uint32_t right_value = right_signed ? static_cast<int16_t>(right_v) : static_cast<uint16_t>(right_v);

  // pop from stack
  uint16_t carry_in = 0;
  if (stack_mode == S_OUT || stack_mode == S_CHK) { // '>' or '<>'
    carry_in = popStack();
  }

  // comparison
  uint32_t result;
  if (left_value < right_value) {
    result = -1; }
  else if (left_value > right_value) {
    result = 1; }
  else if (left_value == right_value) {
    result = 0;
  }

  // push to stack
  if (stack_mode == S_IN || stack_mode == S_CHK) { // '<' or '<>'
    pushStack(result);
  }
}

void PxCPU::call() {

}

void PxCPU::ret() {
  //PC = mainboard->readWord(SP);
  //SP += 2;
}

/* cpu instruction set */

std::function<void()> PxCPU::decode(uint8_t opcode) {
  auto it = instructionSet.find(opcode);
  if (it != instructionSet.end()) {
    auto instruction = it->second;
    return instruction;
  }
  else {
    throw std::runtime_error("Invalid opcode: " + std::to_string(opcode));
  }
}

// [UNARY/BINARY/TRINARY][F][H][OP]
// UNARY/BINARY/TRINARY - 1, 2, or 3 operands
// H, Half - interpret operands as half precision floating point
// F, Function - use a function instead of operator on operands

#define OPCODE(op) [this]() { op; };

#define UNARYOP(op) unaryOp([](uint32_t a) { return op; })
#define UNARYHOP(op) unaryOp([](uint32_t a) { return encodeHalf((op)(decodeHalf(a))); })

#define BINARYOP(op) binaryOp([](uint32_t a, uint32_t b) { return a op b; })
#define BINARYFOP(op) binaryOp([](uint32_t a, uint32_t b) { return op(a,b); })
#define BINARYHOP(op) binaryOp([](uint32_t a, uint32_t b) { return encodeHalf(decodeHalf(a) op decodeHalf(b)); }) // kinda unoptimizatory.
#define BINARYHFOP(op) binaryOp([](uint32_t a, uint32_t b) { return encodeHalf(op(decodeHalf(a),decodeHalf(b))); })

#define TRINARYOP(op) trinaryOp([](uint32_t a, uint32_t b, uint32_t c) { return a op b op c; })

#define CMPOP() cmpOp()
#define JMPOP(op) jmpOp([](int32_t a, int32_t b) { return a op b; })

#define HALT() reset()
#define RST() reset()
#define NOP() nop()
#define PUSH(indirect) push(indirect)
#define POP() pop()
#define CALL(indirect) call(indirect)
#define RET() ret()
#define HALF(op) encodeHalf(op(decodeHalf(a)))

void PxCPU::initializeInstructionSet() {
  instructionSet[0x00] = OPCODE(NOP());                                            // NOP
                                                                                 
  instructionSet[0x01] = OPCODE(BINARYOP(=));                                      // MOV
  instructionSet[0x02] = OPCODE(BINARYOP(=));                                      // SWAP
                                                                                 
  instructionSet[0x20] = OPCODE(BINARYOP(+));                                      // ADD
  instructionSet[0x21] = OPCODE(BINARYOP(-));                                      // SUB
  instructionSet[0x22] = OPCODE(BINARYOP(*));                                      // MUL
  instructionSet[0x23] = OPCODE(BINARYOP(/));                                      // DIV
  instructionSet[0x24] = OPCODE(BINARYOP(%));                                      // MOD
  instructionSet[0x25] = OPCODE(BINARYOP(&));                                      // AND
  instructionSet[0x26] = OPCODE(BINARYOP(^));                                      // XOR
  instructionSet[0x27] = OPCODE(BINARYOP(|));                                      // OR
  instructionSet[0x40] = OPCODE(UNARYOP(~a));                                      // NOT

  instructionSet[0x41] = OPCODE(UNARYOP(a + 1));                                   // INC
  instructionSet[0x42] = OPCODE(UNARYOP(a - 1));                                   // DEC

  instructionSet[0x28] = OPCODE(BINARYOP(<<));                                     // SHL
  instructionSet[0x29] = OPCODE(BINARYOP(>>));                                     // SHR

  instructionSet[0x33] = OPCODE(BINARYOP(<<));                                     // SHL (signed arithmetic left, same as SHL)
  instructionSet[0x34] = OPCODE(BINARYOP(>>));                                     // SAR (signed arithmetic right)

  instructionSet[0x30] = OPCODE(BINARYOP(*));                                      // MULS (signed)
  instructionSet[0x31] = OPCODE(BINARYOP(/ ));                                     // DIVS (signed)
  instructionSet[0x32] = OPCODE(BINARYOP(%));                                      // MODS (signed)
                                                                                 
  instructionSet[0x50] = OPCODE(UNARYOP(intToHalf(a)));                            // HALF
  instructionSet[0x51] = OPCODE(UNARYOP(halfToInt(a)));                            // UHALF

  instructionSet[0x52] = OPCODE(BINARYHOP(+));                                     // HADD
  instructionSet[0x53] = OPCODE(BINARYHOP(-));                                     // HSUB
  instructionSet[0x54] = OPCODE(BINARYHOP(*));                                     // HMUL
  instructionSet[0x55] = OPCODE(BINARYHOP(/ ));                                    // HDIV

  instructionSet[0x58] = OPCODE(UNARYOP(sqrt(a)));                                 // ISQRT
  instructionSet[0x59] = OPCODE(UNARYHOP(sqrt));                                   // SQRT
  instructionSet[0x5A] = OPCODE(BINARYHFOP(atan2));                                // ATAN2
  instructionSet[0x5B] = OPCODE(UNARYHOP(sin));                                    // SIN

  //instructionSet[0x55] = OPCODE(UNARYOP(a));                                     // ASIN
  //instructionSet[0x55] = OPCODE(UNARYOP(a));                                     // COS
  //instructionSet[0x55] = OPCODE(UNARYOP(a));                                     // ACOS
  //instructionSet[0x55] = OPCODE(UNARYOP(a));                                     // TAN
  //instructionSet[0x55] = OPCODE(UNARYOP(a));                                     // ATAN

  //instructionSet[0x55] = OPCODE(UNARYOP(abs(a)));                                // ABS   
  //instructionSet[0x55] = OPCODE(UNARYOP(sqrt(a)); );                             // SQRT  
  //instructionSet[0x55] = OPCODE(UNARYOP(a));                                     // LN    
  //instructionSet[0x55] = OPCODE(UNARYOP(a));                                     // LOG2  
  //instructionSet[0x55] = OPCODE(UNARYOP(a));                                     // LOG10 

  //instructionSet[0x55] = OPCODE(UNARYOP());                                      // RAND
  
  instructionSet[0x60] = OPCODE(CMPOP());                                          // CMP
  //instructionSet[0x61] = OPCODE(CMPOPS());                                       // CMP (signed)
                                                                                   
                                                                                   // JMP = JE.
  instructionSet[0x70] = OPCODE(JMPOP(==));                                        // JE   = JZ 
  instructionSet[0x71] = OPCODE(JMPOP(!=));                                        // JNE  = JNZ
  instructionSet[0x72] = OPCODE(JMPOP(<));                                         // JLT
  instructionSet[0x73] = OPCODE(JMPOP(<=));                                        // JLE
  instructionSet[0x74] = OPCODE(JMPOP(>));                                         // JGT
  instructionSet[0x75] = OPCODE(JMPOP(>=));                                        // JGE
                                                                                   
  instructionSet[0x80] = OPCODE(NOP());                                            // PUSH
  instructionSet[0x81] = OPCODE(NOP());                                            // POP
  instructionSet[0x82] = OPCODE(CALL());                                           // CALL
  instructionSet[0x83] = OPCODE(RET());                                            // RET
  
}
