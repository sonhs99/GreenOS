#include "PIC.hpp"
#include "Assembly.hpp"

void kInitializePIC() {
    kOutPortByte(PIC_MASTER_PORT1, 0x11);
    kOutPortByte(PIC_MASTER_PORT2, PIC_IRQSTARTVECTOR);

    kOutPortByte(PIC_MASTER_PORT2, 0x04);
    kOutPortByte(PIC_MASTER_PORT2, 0x01);

    kOutPortByte(PIC_SLAVE_PORT1, 0x11);
    kOutPortByte(PIC_SLAVE_PORT2, PIC_IRQSTARTVECTOR + 8);

    kOutPortByte(PIC_SLAVE_PORT2, 0x02);
    kOutPortByte(PIC_SLAVE_PORT2, 0x01);
}

void kMaskPICInterrupt(u16 wIRQBitMask) {
    kOutPortByte(PIC_MASTER_PORT2, u8(wIRQBitMask));
    kOutPortByte(PIC_SLAVE_PORT1, u8(wIRQBitMask >> 8));
}

void kSendEOIToPIC(int iIRQNumber) {
    kOutPortByte(PIC_MASTER_PORT1, 0x20);
    if(iIRQNumber >= 8)
        kOutPortByte(PIC_SLAVE_PORT1, 0x20);
}