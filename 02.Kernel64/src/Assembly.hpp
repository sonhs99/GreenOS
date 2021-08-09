#pragma once

#include "Types.hpp"

extern "C" {
    u8 kInPortByte(u16 wPort);
    void kOutPortByte(u16 wPort, u8 bData);
    void kLoadGDTR(u64 qwGDTRAddress);
    void kLoadTR(u16 wTSSSegmentOffset);
    void kLoadIDTR(u64 qwIDTRAddress);
}