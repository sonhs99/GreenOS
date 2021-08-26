#pragma once

#include "Types.hpp"

extern "C" {
    void kReadCPUID(u32 dwEAX, u32* pdwEAX, u32* pdwEBX, u32* pdwECX, u32* pdwEDX);
    void kSwitchAndExecute64BitKernel();
}

