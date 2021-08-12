#pragma once

#include "Types.hpp"

#define PIC_MASTER_PORT1    0x20
#define PIC_MASTER_PORT2    0x21
#define PIC_SLAVE_PORT1     0xA0
#define PIC_SLAVE_PORT2     0xA1

#define PIC_IRQSTARTVECTOR  0x20

void kInitializePIC();
void kMaskPICInterrupt(u16 wIRQBitMast);
void kSendEOIToPIC(int iIRQNumber);