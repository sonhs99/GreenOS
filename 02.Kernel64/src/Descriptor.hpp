#pragma once

#include "Types.hpp"

#define GDT_TYPE_CODE           0x0A
#define GDT_TYPE_DATA           0x02
#define GDT_TYPE_TSS            0x09
#define GDT_FLAGS_LOWER_S       0x10
#define GDT_FLAGS_LOWER_DPL0    0x00
#define GDT_FLAGS_LOWER_DPL1    0x20
#define GDT_FLAGS_LOWER_DPL2    0x40
#define GDT_FLAGS_LOWER_DPL3    0x60
#define GDT_FLAGS_LOWER_P       0x80
#define GDT_FLAGS_UPPER_L       0x20
#define GDT_FLAGS_UPPER_DB      0x40
#define GDT_FLAGS_UPPER_G       0x80

#define GDT_FLAGS_LOWER_KERNELCODE  (GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_KERNELDATA  (GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_TSS         (                                    GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_USERCODE    (GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_USERDATA    (GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P)

#define GDT_FLAGS_UPPER_CODE        (GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L)
#define GDT_FLAGS_UPPER_DATA        (GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L)
#define GDT_FLAGS_UPPER_TSS         (GDT_FLAGS_UPPER_G                    )

#define GDT_KERNELCODESEGMENT   0x08
#define GDT_KERNELDATASEGMENT   0x10
#define GDT_TSSSEGMENT          0x18

#define GDTR_STARTADDRESS       0x142000
#define GDT_MAXENTRY8COUNT      3
#define GDT_MAXENTRY16COUNT     1
#define GDT_TABLESIZE           ((sizeof(GDTEntry8) * GDT_MAXENTRY8COUNT) + (sizeof(GDTEntry16) * GDT_MAXENTRY16COUNT))
#define TSS_SEGMENTSIZE         (sizeof(TSSSegment))

#define IDT_TYPE_INTERRUPT      0x0E
#define IDT_TYPE_TRAP           0x0F
#define IDT_FLAGS_DPL0          0x00
#define IDT_FLAGS_DPL1          0x20
#define IDT_FLAGS_DPL2          0x40
#define IDT_FLAGS_DPL3          0x60
#define IDT_FLAGS_P             0x80
#define IDT_FLAGS_IST0          0
#define IDT_FLAGS_IST1          1

#define IDT_FLAGS_KERNEL        (IDT_FLAGS_DPL0 | IDT_FLAGS_P)
#define IDT_FLAGS_USER          (IDT_FLAGS_DPL3 | IDT_FLAGS_P)

#define IDT_MAXENTRYCOUNT       100
#define IDTR_STARTADDRESS       (GDTR_STARTADDRESS + sizeof(GDTR) + GDT_TABLESIZE + TSS_SEGMENTSIZE)
#define IDT_STARTADDRESS        (IDTR_STARTADDRESS + sizeof(IDTR))

#define IDT_TABLESIZE           (IDT_MAXENTRYCOUNT * sizeof(IDTEntry))
#define IST_STARTADDRESS        0x700000
#define IST_SIZE                0x100000

#pragma pack(push, 1)

struct GDTR {
    u16 wLimit;
    u64 qwBaseAddress;
    u16 wPadding;
    u32 dwPadding;
};
using IDTR = GDTR;

struct GDTEntry8 {
    u16 wLowerLimit;
    u16 wLowerBaseAddress;
    u8  bUpperBaseAddress1;
    u8  bTypesAndLowerFlag;
    u8  bUpperLimitAndUpperFlag;
    u8  bUpperBaseAddress2;
    void set(u32 dwBaseAddress, u32 dwLimit, u8 bUpperFlags, u8 bLowerFlags, u8 bTypes);
};

struct GDTEntry16 {
    u16 wLowerLimit;
    u16 wLowerBaseAddress;
    u8  bMiddleBaseAddress1;
    u8  bTypeAndLowerFlags;
    u8  bUpperLimitAndUpperFlag;
    u8  bMiddleBaseAddress2;
    u32 dwUpperBaseAddress;
    u32 dwReserved;
    void set(u64 dwBaseAddress, u32 dwLimit, u8 bUpperFlags, u8 bLowerFlags, u8 bTypes);
};

struct TSSSegment {
    u32 dwReserved1;
    u64 qwRsp[3];
    u64 qwReserved2;
    u64 qwIST[7];
    u64 qwReserved3;
    u16 wReserverd;
    u16 wIOMapBaseAddress;
    void init();
};

struct IDTEntry {
    u16 wLowerBaseAddress;
    u16 wSegmentSelector;
    u8  bIST;
    u8  bTypeAndFlags;
    u16 wMiddleBaseAddress;
    u32 dwUpperBaseAddress;
    u32 dwReserved;
    void set(void* pvHandler, u16 wSelector, u8 bIST, u8 bFlag, u8 bType);
};

#pragma pack(pop)

void kInitializeGDTTableAndTSS();
void kInitializeIDTTables();
