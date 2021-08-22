#include "Types.hpp"
#include "Keyboard.hpp"
#include "Descriptor.hpp"
#include "Assembly.hpp"
#include "PIC.hpp"
#include "Console.hpp"
#include "ConsoleShell.hpp"
#include "Utility.hpp"

extern "C" void Main() {
	int iCursorX, iCursorY;

	kInitializeConsole(0, 10);
	kPrintf("Switch To IA-32e Mode.......................[Pass]\n");
    kPrintf("IA-32e C++ Language Kernel Start............[Pass]\n");
	kPrintf("Initialize Console..........................[Pass]\n");

	kGetCursor(iCursorX, iCursorY);
	kPrintf("GDT Initialize And Switch For IA-32e Mode...[    ]");
    kInitializeGDTTableAndTSS();
    kLoadGDTR(GDTR_STARTADDRESS);
	kSetCursor(45, iCursorY++);
    kPrintf("Pass\n");

	kPrintf("TSS Segment Load............................[    ]");
    kLoadTR(GDT_TSSSEGMENT);
	kSetCursor(45, iCursorY++);
	kPrintf("Pass\n");

    kPrintf("IDT Initialize..............................[    ]");
    kInitializeIDTTables();
    kLoadIDTR(IDTR_STARTADDRESS);
	kSetCursor(45, iCursorY++);
	kPrintf("Pass\n");

	kPrintf("Total RAM Check.............................[    ]");
	kCheckTotalRAMSize();
	kSetCursor(45, iCursorY++);
	kPrintf("Pass], Size = %d MB \n", kGetTotalRAMSize());

	kPrintf("Keyboard Activate And Queue Initialize......[    ]");
	kSetCursor(45, iCursorY++);
	if(kInitializeKeyboard()){
        kPrintf("Pass\n");
        kChangeKeyboardLED(false, false, false);
    } else {
        kPrintf("Fail\n");
        while(true);
    }

    kPrintf("PIC Controller And Interrupt Initialize.....[    ]");
	kSetCursor(45, iCursorY++);
	kInitializePIC();
    kMaskPICInterrupt(0);
    kEnableInterrupt();
    kPrintf("Pass\n");

	kStartConsoleShell();
}