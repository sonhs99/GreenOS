#pragma once

#include "Types.hpp"
#include "Task.hpp"

extern "C" {
    u8   kInPortByte(u16 wPort);
    void kOutPortByte(u16 wPort, u8 bData);
    void kLoadGDTR(u64 qwGDTRAddress);
    void kLoadTR(u16 wTSSSegmentOffset);
    void kLoadIDTR(u64 qwIDTRAddress);
    void kEnableInterrupt();
    void kDisableInterrupt();
    u64  kReadFLAGS();
    u64  kReadTSC();
    void kSwitchContext(Context *pstCurrentContext, Context *pstNextContext);
}