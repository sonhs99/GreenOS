#include "Types.hpp"
#include "Keyboard.hpp"
#include "Descriptor.hpp"
#include "Assembly.hpp"
#include "PIC.hpp"

void kPrintString(int iX, int iY, const char * pcString);
extern "C" void Main() {
    char vcTemp[2] = {0, 0};
    int i = 0;
    KeyData stData;

    kPrintString( 0, 10, "Switch To IA-32e Mode.......................[Pass]");
    kPrintString( 0, 11, "IA-32e C++ Language Kernel Start............[Pass]");

    kPrintString( 0, 12, "GDT Initialize And Switch For IA-32e Mode...[    ]");
    kInitializeGDTTableAndTSS();
    kLoadGDTR(GDTR_STARTADDRESS);
    kPrintString(45, 12, "Pass");
        
    kPrintString( 0, 13, "TSS Segment Load............................[    ]");
    kLoadTR(GDT_TSSSEGMENT);
    kPrintString(45, 13, "Pass");

    kPrintString( 0, 14, "IDT Initialize..............................[    ]");
    kInitializeIDTTables();
    kLoadIDTR(IDTR_STARTADDRESS);
    kPrintString(45, 14, "Pass");

    kPrintString( 0, 15, "Keyboard Activate And Queue Initialize......[    ]");
    if(kInitializeKeyboard()){
        kPrintString( 45, 15, "Pass");
        kChangeKeyboardLED(false, false, false);
    } else {
        kPrintString( 45, 15, "Fail");
        while(true);
    }

    kPrintString( 0, 16, "PIC Controller And Interrupt Initialize.....[    ]");
    kInitializePIC();
    kMaskPICInterrupt(0);
    kEnableInterrupt();
    kPrintString( 45, 16, "Pass");

    while(true) {
        if(kGetKeyFromKeyQueue(stData)){
            if(stData.bFlags & KEY_FLAGS_DOWN) {
                vcTemp[0] = stData.bASCIICode;
                kPrintString(i++, 17, vcTemp);
                if(vcTemp[0] == '0') vcTemp[0] /= 0;
            }
        }
    }
}

void kPrintString(int iX, int iY, const char * pcString){
    Charactor* pstScreen = (Charactor*) 0xB8000;
    pstScreen += (iY * 80) + iX;
    for(int i = 0; pcString[i] != 0; i++)
        pstScreen[i].bCharactor = pcString[i];
}