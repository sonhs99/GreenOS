#include "Types.hpp"
#include "Keyboard.hpp"
#include "Descriptor.hpp"
#include "Assembly.hpp"

void kPrintString(int iX, int iY, const char * pcString);
extern "C" void Main() {
    char vcTemp[2] = {0, 0};
    u8 bFlags;
    int i = 0;
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

    kPrintString( 0, 15, "Keyboard Activate...........................[    ]");
    if(kActivateKeyboard()){
        kPrintString( 45, 15, "Pass");
        kChangeKeyboardLED(false, false, false);
    } else {
        kPrintString( 45, 15, "Fail");
        while(true);
    }

    while(true) {
        if(kIsOutputBufferFull()){
            u8 bTemp = kGetKeyboardScanCode();
            if(kConvertScanCodeToASCIICode(bTemp, (u8&)(vcTemp[0]), bFlags)){
                if(bFlags & KEY_FLAGS_DOWN) kPrintString(i++, 16, vcTemp);
                if(vcTemp[0] == '0') bTemp /= 0;
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