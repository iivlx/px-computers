#include <windows.h>

#include <gl/GL.h>

#include "MainWindow.h"
#include "MainScreen.h"

#include "pxCPU.h"
#include "pxClock.h"
#include "pxDisplay.h"
#include "pxMainboard.h"
#include "pxRAM.h"
#include "pxROM.h"

std::pair<uint8_t*, size_t> PxAssemble(std::string code) {
  // returns some assembly code...
  uint8_t test_code[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  size_t code_size = sizeof(test_code);
  uint8_t* machine_code = new uint8_t[code_size];
  for (size_t i = 0; i < code_size; ++i) {
    machine_code[i] = test_code[i];
  }
  return { machine_code, code_size };
};

#ifdef SUBSYSTEM_WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#endif
#ifdef SUBSYSTEM_CONSOLE
int main(int argc, char* argv[]) {
#endif
  MainWindow mainWindow(512, 512, "PxComputers");
  MainScreen screen(&mainWindow, 64, 64);
  



  PxMainboard* mainboard = new PxMainboard();
  PxCPU* cpu = new PxCPU(mainboard);
  PxClock* clock = new PxClock(100); // 100 Hz clock
  PxRAM* ram = new PxRAM(0x8000); // 32KB  RAM
  PxRAM* ram2 = new PxRAM(0x1E00); // 7,680 bytes secondary RAM
  PxROM* rom = new PxROM(0x100); // 256 bytes ROM
  PxDisplay* display = new PxDisplay(64, 64, true, 0.5f); // 64x64 display - 24,832 bytes

  mainboard->addCPU(cpu);

  mainboard->addDevice("Display", display, 0x8000, 0x6100); // 8000 -> E0FF
  mainboard->addDevice("Clock", clock, 0xE100, 0x100); // E100 -> E1FF
  mainboard->addDevice("RAM1", ram, 0x0000, 0x8000); // 0000 -> 7FFF
  mainboard->addDevice("RAM2", ram2, 0xE200, 0x1D00); // E200 -> FEFF
  mainboard->addDevice("ROM", rom, 0xFF00, 0x100); // FF00 -> FFFF

  mainboard->main_display = display;
  mainboard->setClock(clock); // synchronize clock with CPU
  mainboard->setResetVector(mainboard->devices[rom].first); // start in ROM

  auto [startup_code, startup_size] = PxAssemble(
      "jmp ROM:0" // loop from start of ROM
  );

  rom->writeBytes(0x00, startup_code, startup_size);

  PxMainboard* mainboard2 = new PxMainboard();
  {
    PxCPU* cpu = new PxCPU(mainboard2);
    PxClock* clock = new PxClock(100); // 100 Hz clock
    PxRAM* ram = new PxRAM(0x8000); // 32KB  RAM
    PxRAM* ram2 = new PxRAM(0x1E00); // 7,680 bytes secondary RAM
    PxROM* rom = new PxROM(0x100); // 256 bytes ROM
    PxDisplay* display = new PxDisplay(64, 64, true); // 64x64 display - 24,832 bytes

    mainboard2->addCPU(cpu);

    mainboard2->addDevice("Display", display, 0x8000, 0x6100); // 8000 -> E0FF
    mainboard2->addDevice("Clock", clock, 0xE100, 0x100); // E100 -> E1FF
    mainboard2->addDevice("RAM1", ram, 0x0000, 0x8000); // 0000 -> 7FFF
    mainboard2->addDevice("RAM2", ram2, 0xE200, 0x1D00); // E200 -> FEFF
    mainboard2->addDevice("ROM", rom, 0xFF00, 0x100); // FF00 -> FFFF

    mainboard2->main_display = display;
    mainboard2->setClock(clock); // synchronize clock with CPU
    //mainboard2->setResetVector(mainboard2->devices[rom].first); // start in ROM
    mainboard2->setResetVector(0xFF10); // start in ROM

    // hardcode program in ROM
    uint8_t code[] = {
      /* data:     */ 
      0x80, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      /*_start: */
      /* 0xff20: */ 0x00, 0x00, 0x00,                                 /* NOP  */
      /* 0xff23: */ 0x00, 0x00, 0x00,                                 /* NOP  */
      /* 0xff26: */ 0x01, 0x00, 0x20, 0x00, 0x10, 0x00, 0x00,         /* MOV [t], 0 */
      /*Loop: */
      /* 0xff2d: */ 0x01, 0x00, 0x20, 0x00, 0x14, 0x00, 0x00,         /* MOV [Y], 0 */
      /*LoopY: */
      /* 0xff34: */ 0x01, 0x00, 0x20, 0x00, 0x12, 0x00, 0x00,         /* MOV [X], 0 */
      /*LoopX: */
      /* 0xff3b: */ 0x01, 0x00, 0x22, 0x00, 0x16, 0x00, 0x14,         /* MOV [P], [Y] */
      /* 0xff42: */ 0x22, 0x00, 0x20, 0x00, 0x16, 0x00, 0x40,         /* MUL [P], 64 */
      /* 0xff49: */ 0x20, 0x00, 0x22, 0x00, 0x16, 0x00, 0x12,         /* ADD [P], [X] */
      /* 0xff50: */ 0x22, 0x00, 0x20, 0x00, 0x16, 0x00, 0x03,         /* MUL [P], 3 */
      /* 0xff57: */ 0x20, 0x00, 0x20, 0x00, 0x16, 0x80, 0x00,         /* ADD [P], 0x8000 */
      /* 0xff5e: */ 0x01, 0x00, 0x22, 0x00, 0x30, 0x00, 0x10,         /* MOV [S], [t] */
      /* 0xff65: */ 0x50, 0x00, 0x20, 0x00, 0x30,                     /* HALF [S] */
      /* 0xff6a: */ 0x52, 0x00, 0x20, 0x00, 0x30, 0x3E, 0x00,         /* HMUL [S], 1.5 */
      /* 0xff71: */ 0x51, 0x00, 0x20, 0x00, 0x30,                     /* UHALF [S] */
      /* 0xff76: */ 0x01, 0x00, 0x42, 0x00, 0x16, 0x00, 0x30,         /* MOV (P), [S] */
      /* 0xff7d: */ 0x41, 0x00, 0x20, 0x00, 0x12,                     /* INC [X] */
      /* 0xff82: */ 0x60, 0x02, 0x20, 0x00, 0x12, 0x00, 0x40,         /* CMP< [X], 64 */
      /* 0xff89: */ 0x72, 0x01, 0x00, 0xFF, 0x3B,                     /* JLT> LoopX */
      /* 0xff8e: */ 0x41, 0x00, 0x20, 0x00, 0x14,                     /* INC [Y] */
      /* 0xff93: */ 0x60, 0x02, 0x20, 0x00, 0x14, 0x00, 0x40,         /* CMP< [Y], 64 */
      /* 0xff9a: */ 0x72, 0x01, 0x00, 0xFF, 0x34,                     /* JLT> LoopY */
      /* 0xff9f: */ 0x41, 0x00, 0x20, 0x00, 0x10,                     /* INC [t] */
      /* 0xffa4: */ 0x60, 0x02, 0x20, 0x00, 0x10, 0x00, 0xFF,         /* CMP< [t], 255 */
      /* 0xffab: */ 0x72, 0x01, 0x00, 0xFF, 0x2D,                     /* JLT> Loop */
      /* 0xffb0: */ 0x70, 0x00, 0x00, 0xFF, 0x20,                     /* JMP _start */
    };

    rom->writeBytes(0x0000, code, sizeof(code));


    screen.addDisplay(display);
  }





  screen.addDisplay(display);
  mainWindow.run(screen, std::vector<PxMainboard*>{mainboard2});

  return 0;

}



