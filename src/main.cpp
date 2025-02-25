#include <windows.h>

#include <gl/GL.h>

#include "Window.h"
#include "Screen.h"

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
  PxKeyboard* keyboard = new PxKeyboard(0x64); // 100 key keyboard
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

  // hardcode program in ROM
  uint8_t code[] = {
    /* data:     */ 
    0x80, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*_start: */
    /* 0xfe20: */ 0x00, 0x00, 0x00,                                 /* NOP  */
    /* 0xfe23: */ 0x00, 0x00, 0x00,                                 /* NOP  */
    /* 0xfe26: */ 0x01, 0x00, 0x20, 0x00, 0x10, 0x00, 0x00,         /* MOV [t], 0 */
    /*Loop: */
    /* 0xfe2d: */ 0x01, 0x00, 0x20, 0x00, 0x14, 0x00, 0x00,         /* MOV [Y], 0 */
    /*LoopY: */
    /* 0xfe34: */ 0x01, 0x00, 0x20, 0x00, 0x12, 0x00, 0x00,         /* MOV [X], 0 */
    /*LoopX: */
    /* 0xfe3b: */ 0x01, 0x00, 0x22, 0x00, 0x16, 0x00, 0x14,         /* MOV [P], [Y] */
    /* 0xfe42: */ 0x22, 0x00, 0x20, 0x00, 0x16, 0x00, 0x40,         /* MUL [P], 64 */
    /* 0xfe49: */ 0x20, 0x00, 0x22, 0x00, 0x16, 0x00, 0x12,         /* ADD [P], [X] */
    /* 0xfe50: */ 0x22, 0x00, 0x20, 0x00, 0x16, 0x00, 0x03,         /* MUL [P], 3 */
    /* 0xfe57: */ 0x20, 0x00, 0x20, 0x00, 0x16, 0x80, 0x00,         /* ADD [P], Display */
    /* 0xfe5e: */ 0x01, 0x00, 0x22, 0x00, 0x20, 0x00, 0x12,         /* MOV [Dx], [X] */
    /* 0xfe65: */ 0x21, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20,         /* SUB [Dx], CENTER_X */
    /* 0xfe6c: */ 0x01, 0x00, 0x22, 0x00, 0x22, 0x00, 0x14,         /* MOV [Dy], [Y] */
    /* 0xfe73: */ 0x21, 0x00, 0x20, 0x00, 0x22, 0x00, 0x20,         /* SUB [Dy], CENTER_Y */
    /* 0xfe7a: */ 0x30, 0x00, 0x22, 0x00, 0x20, 0x00, 0x20,         /* MULS [Dx], [Dx] */
    /* 0xfe81: */ 0x30, 0x00, 0x22, 0x00, 0x22, 0x00, 0x22,         /* MULS [Dy], [Dy] */
    /* 0xfe88: */ 0x01, 0x00, 0x22, 0x00, 0x24, 0x00, 0x20,         /* MOV [Dist], [Dx] */
    /* 0xfe8f: */ 0x20, 0x00, 0x22, 0x00, 0x24, 0x00, 0x22,         /* ADD [Dist], [Dy] */
    /* 0xfe96: */ 0x50, 0x00, 0x20, 0x00, 0x24,                     /* HALF [Dist] */
    /* 0xfe9b: */ 0x59, 0x00, 0x20, 0x00, 0x24,                     /* SQRT [Dist] */
    /* 0xfea0: */ 0x01, 0x00, 0x22, 0x00, 0x20, 0x00, 0x12,         /* MOV [Dx], [X] */
    /* 0xfea7: */ 0x21, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20,         /* SUB [Dx], CENTER_X */
    /* 0xfeae: */ 0x01, 0x00, 0x22, 0x00, 0x22, 0x00, 0x14,         /* MOV [Dy], [Y] */
    /* 0xfeb5: */ 0x21, 0x00, 0x20, 0x00, 0x22, 0x00, 0x20,         /* SUB [Dy], CENTER_Y */
    /* 0xfebc: */ 0x50, 0x00, 0x20, 0x00, 0x22,                     /* HALF [Dy] */
    /* 0xfec1: */ 0x50, 0x00, 0x20, 0x00, 0x20,                     /* HALF [Dx] */
    /* 0xfec6: */ 0x01, 0x00, 0x22, 0x00, 0x26, 0x00, 0x22,         /* MOV [Angle], [Dy] */
    /* 0xfecd: */ 0x5A, 0x00, 0x22, 0x00, 0x26, 0x00, 0x20,         /* ATAN2 [Angle], [Dx] */
    /* 0xfed4: */ 0x54, 0x00, 0x20, 0x00, 0x26, 0x41, 0x00,         /* HMUL [Angle], 1.5 */
    /* 0xfedb: */ 0x01, 0x00, 0x22, 0x00, 0x30, 0x00, 0x10,         /* MOV [S], [t] */
    /* 0xfee2: */ 0x50, 0x00, 0x20, 0x00, 0x30,                     /* HALF [S] */
    /* 0xfee7: */ 0x54, 0x00, 0x20, 0x00, 0x30, 0x32, 0x66,         /* HMUL [S], 0.2 */
    /* 0xfeee: */ 0x52, 0x00, 0x22, 0x00, 0x30, 0x00, 0x24,         /* HADD [S], [Dist] */
    /* 0xfef5: */ 0x53, 0x00, 0x22, 0x00, 0x30, 0x00, 0x26,         /* HSUB [S], [Angle] */
    /* 0xfefc: */ 0x5B, 0x00, 0x20, 0x00, 0x30,                     /* SIN [S] */
    /* 0xff01: */ 0x54, 0x00, 0x20, 0x00, 0x30, 0x57, 0xF0,         /* HMUL [S], 127.0 */
    /* 0xff08: */ 0x52, 0x00, 0x20, 0x00, 0x30, 0x58, 0x00,         /* HADD [S], 128.0 */
    /* 0xff0f: */ 0x51, 0x00, 0x20, 0x00, 0x30,                     /* UHALF [S] */
    /*Draw: */
    /* 0xff14: */ 0x01, 0x00, 0x22, 0x00, 0x28, 0x00, 0x30,         /* MOV [Color], [S] */
    /* 0xff1b: */ 0x01, 0x00, 0x52, 0x00, 0x16, 0x00, 0x28,         /* MOV (P), [Color] */
    /* 0xff22: */ 0x01, 0x00, 0x22, 0x00, 0x28, 0x00, 0x30,         /* MOV [Color], [S] */
    /* 0xff29: */ 0x23, 0x00, 0x20, 0x00, 0x28, 0x00, 0x02,         /* DIV [Color], 2 */
    /* 0xff30: */ 0x41, 0x00, 0x20, 0x00, 0x16,                     /* INC [P] */
    /* 0xff35: */ 0x01, 0x00, 0x52, 0x00, 0x16, 0x00, 0x28,         /* MOV (P), [Color] */
    /*Next: */
    /* 0xff3c: */ 0x41, 0x00, 0x20, 0x00, 0x12,                     /* INC [X] */
    /* 0xff41: */ 0x60, 0x02, 0x20, 0x00, 0x12, 0x00, 0x40,         /* CMP< [X], 64 */
    /* 0xff48: */ 0x72, 0x01, 0x00, 0xFE, 0x3B,                     /* JLT> LoopX */
    /* 0xff4d: */ 0x41, 0x00, 0x20, 0x00, 0x14,                     /* INC [Y] */
    /* 0xff52: */ 0x60, 0x02, 0x20, 0x00, 0x14, 0x00, 0x40,         /* CMP< [Y], 64 */
    /* 0xff59: */ 0x72, 0x01, 0x00, 0xFE, 0x34,                     /* JLT> LoopY */
    /* 0xff5e: */ 0x41, 0x00, 0x20, 0x00, 0x10,                     /* INC [t] */
    /* 0xff63: */ 0x60, 0x02, 0x20, 0x00, 0x10, 0x00, 0xFF,         /* CMP< [t], 255 */
    /* 0xff6a: */ 0x72, 0x01, 0x00, 0xFE, 0x2D,                     /* JLT> Loop */
    /* 0xff6f: */ 0x70, 0x00, 0x00, 0xFE, 0x20,                     /* JMP _start */
  };
  rom->writeBytes(0x0000, code, sizeof(code));

  // add virtual computer's display to the screen and run
  screen.addDevice(display);
  mainWindow.run(screen, std::vector<PxMainboard*>{mainboard});

  return 0;
}
