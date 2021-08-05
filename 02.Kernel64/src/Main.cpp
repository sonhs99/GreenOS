#include "Types.hpp"
#include "Keyboard.hpp"

void kPrintString(int iX, int iY, const char * pcString);
extern "C"{
    void Main() {
        char vcTemp[2] = {0, 0};
        u8 bFlags;
        int i = 0;
        kPrintString( 0, 10, "Switch To IA-32e Mode.......................[Pass]");
        kPrintString( 0, 11, "IA-32e C++ Language Kernel Start............[Pass]");

        kPrintString( 0, 12, "Keyboard Activate...........................[    ]");
        if(kActivateKeyboard()){
            kPrintString( 45, 12, "Pass");
            kChangeKeyboardLED(false, false, false);
        } else {
            kPrintString( 45, 12, "Fail");
            while(true);
        }

        while(true) {
            if(kIsOutputBufferFull()){
                u8 bTemp = kGetKeyboardScanCode();
                if(kConvertScanCodeToASCIICode(bTemp, (u8&)(vcTemp[0]), bFlags)){
                    kPrintString(i++, 13, "A");
                    if(bFlags & KEY_FLAGS_DOWN) kPrintString(i++, 13, vcTemp);}
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