#pragma once

#include "Types.hpp"

#define CONSOLE_BACKGROUND_BLACK            0x00
#define CONSOLE_BACKGROUND_BLUE             0x10
#define CONSOLE_BACKGROUND_GREEN            0x20
#define CONSOLE_BACKGROUND_CYAN             0x30
#define CONSOLE_BACKGROUND_RED              0x40
#define CONSOLE_BACKGROUND_MAGENTA          0x50
#define CONSOLE_BACKGROUND_BROWN            0x60
#define CONSOLE_BACKGROUND_WHITE            0x70

#define CONSOLE_BACKGROUND_BLINK            0x80

#define CONSOLE_FOREGROUND_BLACK            0x00
#define CONSOLE_FOREGROUND_BLUE             0x01
#define CONSOLE_FOREGROUND_GREEN            0x02
#define CONSOLE_FOREGROUND_CYAN             0x03
#define CONSOLE_FOREGROUND_RED              0x04
#define CONSOLE_FOREGROUND_MAGENTA          0x05
#define CONSOLE_FOREGROUND_BROWN            0x06
#define CONSOLE_FOREGROUND_WHITE            0x07

#define CONSOLE_FOREGROUND_BRIGHT           0x08

#define CONSOLE_DEFAULTTEXTCOLOR            (CONSOLE_BACKGROUND_BLACK | CONSOLE_FOREGROUND_GREEN | CONSOLE_FOREGROUND_BRIGHT)

#define CONSOLE_WIDTH                       80
#define CONSOLE_HEIGHT                      25
#define CONSOLE_VIDEOMEMORYADDRESS          0xB8000

#define VGA_PORT_INDEX                      0x3D4
#define VGA_PORT_DATA                       0x3D5
#define VGA_INDEX_UPPERCURSOR               0x0E
#define VGA_INDEX_LOWERCURSOR               0x0F

#pragma pack(push, 1)

struct ConsoleManager {
    int iCurrentPrintOffset;
};

#pragma pack(pop)

void kInitializeConsole(int iX, int iY);
void kSetCursor(int iX, int iY);
void kGetCursor(int &piX, int &piY);
void kPrintf(const char* pcFormatString, ...);
int  kConsolePrintString(const char * pcBuffer);
void kClearScrenn();
u8   kGetCh();
void kPrintStringXY(int iX, int iY, const char* pcString);

