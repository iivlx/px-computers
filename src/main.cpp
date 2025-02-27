#include <windows.h>

#include <gl/GL.h>

#include "Window.h"
#include "Screen.h"

#include "pxAssemble.h"
#include "PxCPU.h"
#include "PxClock.h"
#include "PxDisplay.h"
#include "PxKeyboard.h"
#include "PxMainboard.h"
#include "PxRAM.h"
#include "PxROM.h"

#ifdef SUBSYSTEM_WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#endif

#ifdef SUBSYSTEM_CONSOLE
int main(int argc, char* argv[]) {
#endif

  // Initialize display
  Window mainWindow(512, 512, "px-computers");
  Screen screen(&mainWindow, 64, 64);

  // Create virtual computer

  // create devices
  PxMainboard* mainboard = new PxMainboard();
  PxCPU* cpu = new PxCPU(mainboard);
  PxClock* clock = new PxClock(100); // 100 Hz clock
  PxRAM* ram = new PxRAM(0x8000); // 32KB  RAM
  PxRAM* ram2 = new PxRAM(0x1E00); // 7,680 bytes secondary RAM
  PxROM* rom = new PxROM(0x200); // 256 bytes ROM
  PxKeyboard* keyboard = new PxKeyboard(0x64); // 100 byte buffer
  PxDisplay* display = new PxDisplay(64, 64, true, 0.5f); // 64x64 display - 24,832 bytes

  // add devices to mainboard
  mainboard->addCPU(cpu);
  mainboard->addDevice("Display", display, 0x8000, 0x6100); // 8000 -> E0FF
  mainboard->addDevice("Clock", clock, 0xE100, 0x100); // E100 -> E1FF
  mainboard->addDevice("RAM1", ram, 0x0000, 0x8000); // 0000 -> 7FFF
  mainboard->addDevice("RAM2", ram2, 0xE200, 0x1C00); // E200 -> FDFF
  mainboard->addDevice("ROM", rom, 0xFE00, 0x200); // FE00 -> FFFF

  // configure mainboard
  mainboard->main_display = display;
  mainboard->setClock(clock); // synchronize clock with CPU
  mainboard->setResetVector(mainboard->devices[rom].first); // start in ROM

  // assemble and load program in ROM
  auto code = pxAssemble("code/spiral.pa");
  rom->writeBytes(0x0000, code.first, code.second);

  // add virtual computer's display to the screen and run
  screen.addDevice(display);
  mainWindow.run(screen, std::vector<PxMainboard*>{mainboard});

  return 0;
}
