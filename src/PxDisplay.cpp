#include <windows.h>

#include <gl/gl.h>

#include "pxOpenGL.h"

#include "PxDisplay.h"


PxDisplay::PxDisplay(int width, int height, bool doubleBuffering, float scale)
  : width(width), height(height), doubleBuffering(doubleBuffering)
  , buffer1(width* height*3, 0)
  , buffer2(doubleBuffering ? width * height*3 : 0, 0)
  , currentBuffer(&buffer1), backBuffer(doubleBuffering ? &buffer2 : nullptr)
  , redrawRequested(false), bufferSwapRequested(false)
{
  if (width <= 0 || height <= 0) {
    throw std::invalid_argument("Invalid size...");
  }

  // control addresses
  controlReadHandlers[PXDISPLAY::READ] = [this]() { return 1; }; // just always return a 1 when read...
  controlWriteHandlers[PXDISPLAY::CLEAR] = [this](uint8_t value) { clearBuffer(1.0f, 1.0f, 1.0f); };
  controlWriteHandlers[PXDISPLAY::SWAPBUFFERS] = [this](uint8_t value) { std::swap(currentBuffer, backBuffer); };
  controlWriteHandlers[PXDISPLAY::REDRAW] = [this](uint8_t value) { redrawRequested = true; };
  controlWriteHandlers[PXDISPLAY::CRASH] = [this](uint8_t value) { throw std::runtime_error("Invalid instruction"); };

}

PxDisplay::~PxDisplay() {
}

uint8_t PxDisplay::readByte(uint16_t address) const {
  return (*currentBuffer)[address];

}

void PxDisplay::writeByte(uint16_t address, uint8_t value) {

  int display_buffer_size = (width * height * (bitsPerPixel/8));
  int buffers_size = doubleBuffering ? display_buffer_size * 2 : display_buffer_size;

  if (address < display_buffer_size) {
    (*currentBuffer)[address] = value;
  }

  else if (address < display_buffer_size*2 && doubleBuffering) {
    (*backBuffer)[address - display_buffer_size] = value;
  }

  else if (address >= buffers_size) {
    handleControlRegisterWrite(address - buffers_size, value);
  }

  else {
    throw std::out_of_range("Invalid memory write address");
  }
}

void PxDisplay::clearBuffer(int r, int g, int b) {
  for (int i = 0; i < width * height * (bitsPerPixel/8); ++i) {
    (*currentBuffer)[i] = static_cast<uint8_t>(0);
  }
}

void PxDisplay::handleControlRegisterWrite(uint16_t address, uint8_t value) {
  controlWriteHandlers[address](value);
}
