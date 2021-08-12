#include "InterruptHandler.hpp"
#include "PIC.hpp"
#include "Keyboard.hpp"

void kPrintString(int iX, int iY, const char * pStr);

void kCommonExceptionHandler(int iVectorNumber, u64 qwErrorCode) {
    char vcBuffer[3] = {0,};

    vcBuffer[0] = '0' + iVectorNumber / 10;
    vcBuffer[1] = '0' + iVectorNumber % 10;

    kPrintString( 0, 0, "==================================================");
    kPrintString( 0, 1, "                  Exception :                     ");
    kPrintString(29, 1, vcBuffer);
    kPrintString( 0, 2, "==================================================");
    while(true);
}

void kCommonInterruptHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInturrptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;
    vcBuffer[8] = '0' + g_iCommonInturrptCount;

    g_iCommonInturrptCount = (g_iCommonInturrptCount + 1) % 10;

    kPrintString(70, 0, vcBuffer);
    kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}

void kKeyboardHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInturrptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;
    vcBuffer[8] = '0' + g_iKeyboardInturrptCount;

    g_iKeyboardInturrptCount = (g_iKeyboardInturrptCount + 1) % 10;

    kPrintString( 0, 0, vcBuffer);

    if(kIsOutputBufferFull()) {
        u8 bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue(bTemp);
    }
    
    kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}
