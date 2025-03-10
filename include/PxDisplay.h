#pragma once

#include <Windows.h>

#include <vector>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <unordered_map>

#include <GL/gl.h>

#include "PxDevice.h"

enum PXDISPLAY {
  // write addresses
  CLEAR = 0,
  SWAPBUFFERS = 1,
  REDRAW = 2,
  CRASH = 7,

  // read addresses
  READ = 0,
};

class PxDisplay : public PxDevice {
public:
  PxDisplay(int width, int height, bool doubleBuffering = false, float scale = 1.0f);
  ~PxDisplay();

  uint8_t readByte(uint16_t address) const;
  void writeByte(uint16_t address, uint8_t value);

  std::vector<uint8_t>* currentBuffer;
  std::vector<uint8_t>* backBuffer;

  void clearBuffer(int r, int g, int b);

  std::pair<int, int> getSize() { return { width, height }; };
  
  bool bufferSwapRequested;
  bool redrawRequested;
  GLuint computeProgram;
  GLuint textureID;
  GLuint VAO;

private:
  void handleControlRegisterWrite(uint16_t address, uint8_t value);
  std::unordered_map<uint16_t, std::function<uint8_t()>> controlReadHandlers; // callback for operations on read addresses...
  std::unordered_map<uint16_t, std::function<void(uint8_t)>> controlWriteHandlers; // callback for operations on write addresses...

  int width;
  int height;
  bool doubleBuffering;
  std::vector<uint8_t> buffer1;
  std::vector<uint8_t> buffer2;

  int bitsPerPixel = 24;
};
