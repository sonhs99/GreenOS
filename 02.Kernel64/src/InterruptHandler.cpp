#include "InterruptHandler.hpp"
#include "PIC.hpp"
#include "Keyboard.hpp"
#include "Console.hpp"
#include "Utility.hpp"
#include "Task.hpp"
#include "Assembly.hpp"
#include "HardDisk.hpp"

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
    static int g_iCommoninterrptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;
    vcBuffer[8] = '0' + g_iCommoninterrptCount;

    g_iCommoninterrptCount = (g_iCommoninterrptCount + 1) % 10;

    kPrintStringXY(70, 0, vcBuffer);
    kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}

void kKeyboardHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardinterrptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;
    vcBuffer[8] = '0' + g_iKeyboardinterrptCount;

    g_iKeyboardinterrptCount = (g_iKeyboardinterrptCount + 1) % 10;

    kPrintStringXY( 0, 0, vcBuffer);

    if(kIsOutputBufferFull()) {
        u8 bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue(bTemp);
    }
    
    kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}

void kTimerHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT:  , ]";
    static int g_iTimerInterruptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;
    vcBuffer[8] = '0' + g_iTimerInterruptCount;

    g_iTimerInterruptCount = (g_iTimerInterruptCount + 1) % 10;

    kPrintStringXY(70, 0, vcBuffer);
    kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);

    g_qwTickCount++;
    kDecreaseProcessorTime();
    if(kIsProcessorTimeExpired()) kScheduleInInterrupt();
}

void kDeviceNotAvailableHandler(int iVectorNumber) {
    char vcBuffer[] = "[EXC:  , ]";
    static int g_iFPUInterruptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;
    vcBuffer[8] = '0' + g_iFPUInterruptCount;

    g_iFPUInterruptCount = (g_iFPUInterruptCount + 1) % 10;

    kPrintStringXY(0, 0, vcBuffer);

    kClearTS();

    u64 qwLastFPUTaskID = kGetLastFPUUsedTaskID();
    Task *pstCurrentTask = kGetRunningTask();

    if(qwLastFPUTaskID == pstCurrentTask->qwID) return;
    else if(qwLastFPUTaskID != TASK_INVALIDID) {
        Task *pstFPUTask = kGetTaskInTCBPool(GETTCBOFFSET(qwLastFPUTaskID));
        if(pstFPUTask != nullptr && pstFPUTask->qwID == qwLastFPUTaskID)
            kSaveFPUContext(pstFPUTask->vqwFPUContext);
    }

    if(!pstCurrentTask->bFPUUsed) {
        kInitializeFPU();
        pstCurrentTask->bFPUUsed = true;
    } else kLoadFPUContext(pstCurrentTask->vqwFPUContext);
    kSetLastFPUUsedTaskID(pstCurrentTask->qwID);
}

void kHDDHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT:  , ]";
    static int g_iHDDInterruptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;
    vcBuffer[8] = '0' + g_iHDDInterruptCount;

    g_iHDDInterruptCount = (g_iHDDInterruptCount + 1) % 10;

    kPrintStringXY(10, 0, vcBuffer);

    if(iVectorNumber - PIC_IRQSTARTVECTOR == 14)
        kSetHDDInterruptFlag(true, true);
    else kSetHDDInterruptFlag(false, true);

    kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}