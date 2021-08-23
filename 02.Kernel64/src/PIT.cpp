#include "PIT.hpp"
#include "Assembly.hpp"

void kInitializePIT(u16 wCount, bool bPeriodic) {
    kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_ONCE);
    if(bPeriodic) kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC);
    kOutPortByte(PIT_PORT_COUNTER0, wCount);
    kOutPortByte(PIT_PORT_COUNTER0, wCount >> 8);
}

u16 kReadCounter0() {
    kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_LATCH);
    u8 bLowByte = kInPortByte(PIT_PORT_COUNTER0);
    u8 bHighByte = kInPortByte(PIT_PORT_COUNTER0);

    u16 wTemp = bLowByte;
    wTemp |= bHighByte << 8;
    return wTemp;
}

void kWaitUsingDirectPIT(u16 wCount) {
    kInitializePIT(0, true);
    u16 wLastCounter0 = kReadCounter0();
    while(true) {
        u16 wCurrentCounter0 = kReadCounter0();
        if(((wLastCounter0 - wCurrentCounter0) & 0xFFFF) >= wCount) break;
    }
}