#include "InterruptHandler.hpp"
#include "PIC.hpp"
#include "Keyboard.hpp"
#include "Console.hpp"

void kCommonExceptionHandler(int iVectorNumber, u64 qwErrorCode) {
    char vcBuffer[3] = {0,};

    vcBuffer[0] = '0' + iVectorNumber / 10;
    vcBuffer[1] = '0' + iVectorNumber % 10;

    kPrintStringXY( 0, 0, "==================================================");
    kPrintStringXY( 0, 1, "                  Exception :                     ");
    kPrintStringXY(29, 1, vcBuffer);
    kPrintStringXY( 0, 2, "==================================================");
    while(true);
}

void kCommonInterruptHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInturrptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;
    vcBuffer[8] = '0' + g_iCommonInturrptCount;

    g_iCommonInturrptCount = (g_iCommonInturrptCount + 1) % 10;

    kPrintStringXY(70, 0, vcBuffer);
    kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}

void kKeyboardHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInturrptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;
    vcBuffer[8] = '0' + g_iKeyboardInturrptCount;

    g_iKeyboardInturrptCount = (g_iKeyboardInturrptCount + 1) % 10;

    kPrintStringXY( 0, 0, vcBuffer);

    if(kIsOutputBufferFull()) {
        u8 bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue(bTemp);
    }
    
    kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}
