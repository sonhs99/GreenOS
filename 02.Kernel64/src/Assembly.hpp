#pragma once

#include "Types.hpp"

extern "C" {
    u8 kInPortByte(u16 wPort);
    u8 kOutPortByte(u16 wPort, u8 bData);
}