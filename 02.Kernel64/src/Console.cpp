#include <stdarg.h>
#include "Console.hpp"
#include "Keyboard.hpp"
#include "Utility.hpp"
#include "Assembly.hpp"

ConsoleManager gs_stConsoleManager = {};

void kInitializeConsole(int iX, int iY) {
    kMemSet(&gs_stConsoleManager, 0, sizeof(gs_stConsoleManager));
    kSetCursor(iX, iY);
}

void kSetCursor(int iX, int iY) {
    int iLinearValue = iY * CONSOLE_WIDTH + iX;

    kOutPortByte(VGA_PORT_INDEX, VGA_INDEX_UPPERCURSOR);
    kOutPortByte(VGA_PORT_DATA, iLinearValue >> 8);

    kOutPortByte(VGA_PORT_INDEX, VGA_INDEX_LOWERCURSOR);
    kOutPortByte(VGA_PORT_DATA, iLinearValue & 0xFF);

    gs_stConsoleManager.iCurrentPrintOffset = iLinearValue;
}

void kGetCursor(int &piX, int &piY) {
    piX = gs_stConsoleManager.iCurrentPrintOffset % CONSOLE_WIDTH;
    piY = gs_stConsoleManager.iCurrentPrintOffset / CONSOLE_WIDTH;
}

void kPrintf(const char* pcFormatString, ...) {
    va_list ap;
    char vcBuffer[ 1024 ];
    
    va_start(ap, pcFormatString);
    kVSPrintf(vcBuffer, pcFormatString, ap);
    va_end(ap);

    int iNextPrintOffset = kConsolePrintString(vcBuffer);
    kSetCursor(iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH);
}

void kClearScreen() {
	Charactor* pstScreen = (Charactor*) CONSOLE_VIDEOMEMORYADDRESS;
	
	for(int i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++)
		pstScreen[i] = {
			.bCharactor = ' ',
			.bAttribute = CONSOLE_DEFAULTTEXTCOLOR
		};
}

int kConsolePrintString(const char* pcBuffer) {
    Charactor* pstScreen = (Charactor*) CONSOLE_VIDEOMEMORYADDRESS;
    
    int iPrintOffset = gs_stConsoleManager.iCurrentPrintOffset;

    int iLength = kStrLen(pcBuffer);
    for(int i = 0; i < iLength; i++) {
        if(pcBuffer[i] == '\n') iPrintOffset += (CONSOLE_WIDTH - (iPrintOffset % CONSOLE_WIDTH));
        else if(pcBuffer[i] == '\t') iPrintOffset += (8 - (iPrintOffset % 8));
        else {
			pstScreen[iPrintOffset] = {
				.bCharactor = u8(pcBuffer[i]),
				.bAttribute = CONSOLE_DEFAULTTEXTCOLOR
			};
            iPrintOffset++;
        }

        if(iPrintOffset >= (CONSOLE_WIDTH * CONSOLE_HEIGHT)) {
            kMemCpy((void*)CONSOLE_VIDEOMEMORYADDRESS,
                    (void*)(CONSOLE_VIDEOMEMORYADDRESS + CONSOLE_WIDTH * sizeof(Charactor)),
                    (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH * sizeof(Charactor));
            for(int j = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH; j < (CONSOLE_HEIGHT * CONSOLE_WIDTH); j++)
				pstScreen[j] = {
					.bCharactor = ' ',
					.bAttribute = CONSOLE_DEFAULTTEXTCOLOR
				};
            iPrintOffset = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
        }
    }
    return iPrintOffset;
}

u8 kGetCh() {
    KeyData stData;
    while(true) {
        while(!kGetKeyFromKeyQueue(stData));

        if(stData.bFlags & KEY_FLAGS_DOWN) return stData.bASCIICode;
    }
}

void kPrintStringXY(int iX, int iY, const char * pcString){
    Charactor* pstScreen = (Charactor*) CONSOLE_VIDEOMEMORYADDRESS;
    pstScreen += (iY * CONSOLE_WIDTH) + iX;
    for(int i = 0; pcString[i] != 0; i++){
		pstScreen[i] = {
			.bCharactor = u8(pcString[i]),
			.bAttribute = CONSOLE_DEFAULTTEXTCOLOR
		};
    }
}