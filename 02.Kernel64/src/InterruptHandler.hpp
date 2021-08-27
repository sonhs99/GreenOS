#pragma once

#include "Types.hpp"

extern "C" {
    void kCommonExceptionHandler(int iVectorNumber, u64 qwErrorCode);
    void kCommonInterruptHandler(int iVectorNumber);
    void kKeyboardHandler(int iVectorNumber);
    void kTimerHandler(int iVectorNumber);
}
