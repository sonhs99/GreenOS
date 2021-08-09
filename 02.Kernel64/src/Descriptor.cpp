#include "Descriptor.hpp"
#include "Utility.hpp"

void kPrintString(int iX, int iY, const char * pStr);

void kInitializeGDTTableAndTSS() {
    GDTR* pstGDTR = (GDTR*) GDTR_STARTADDRESS;
    GDTEntry8* pstEntry = (GDTEntry8*)(GDTR_STARTADDRESS + sizeof(GDTR));
    TSSSegment* pstTSS = (TSSSegment*)(u64(pstEntry) + GDT_TABLESIZE);

    pstGDTR->wLimit = GDT_TABLESIZE - 1;
    pstGDTR->qwBaseAddress = u64(pstEntry);
    pstEntry[0].set(0, 0, 0, 0, 0);
    pstEntry[1].set(0, 0xFFFFF, GDT_FLAGS_UPPER_CODE, GDT_FLAGS_LOWER_KERNELCODE, GDT_TYPE_CODE);
    pstEntry[2].set(0, 0xFFFFF, GDT_FLAGS_UPPER_DATA, GDT_FLAGS_LOWER_KERNELDATA, GDT_TYPE_DATA);
    ((GDTEntry16*)&pstEntry[3])->set(u64(pstTSS), TSS_SEGMENTSIZE - 1, GDT_FLAGS_UPPER_TSS, GDT_FLAGS_LOWER_TSS, GDT_TYPE_TSS);

    pstTSS->init();
}

void GDTEntry8::set(u32 dwBaseAddress, u32 dwLimit, u8 bUpperFlags, u8 bLowerFlags, u8 bTypes) {
    wLowerLimit = dwLimit & 0xFFFF;
    wLowerBaseAddress = dwBaseAddress & 0xFFFF;
    bUpperBaseAddress1 = (dwBaseAddress >> 16) & 0xFF;
    bTypesAndLowerFlag = bLowerFlags | bTypes;
    bUpperLimitAndUpperFlag = ((dwLimit >> 16) & 0xFF) | bUpperFlags;
    bUpperBaseAddress2 = (dwBaseAddress >> 24) & 0xFF;
}

void GDTEntry16::set(u64 qwBaseAddress, u32 dwLimit, u8 bUpperFlags, u8 bLowerFlags, u8 bTypes) {
    wLowerLimit = dwLimit & 0xFFFF;
    wLowerBaseAddress = qwBaseAddress & 0xFFFF;
    bMiddleBaseAddress1 = (qwBaseAddress >> 16) & 0xFF;
    bTypeAndLowerFlags = bLowerFlags | bTypes;
    bUpperLimitAndUpperFlag = ((dwLimit >> 16) & 0xFF) | bUpperFlags;
    bMiddleBaseAddress2 = (qwBaseAddress >> 24) & 0xFF;
    dwUpperBaseAddress = qwBaseAddress >> 32;
    dwReserved = 0;
}

void TSSSegment::init() {
    kMemSet(this, 0, sizeof(TSSSegment));
    qwIST[0] = IST_STARTADDRESS + IST_SIZE;
    wIOMapBaseAddress = 0xFFFF;
}

void kInitializeIDTTables() {
    IDTR* pstIDTR = (IDTR*) IDTR_STARTADDRESS;
    IDTEntry* pstEntry = (IDTEntry*)(IDTR_STARTADDRESS + sizeof(IDTR));

    pstIDTR->qwBaseAddress = u64(pstEntry);
    pstIDTR->wLimit = IDT_TABLESIZE - 1;

    for(int i = 0; i < IDT_MAXENTRYCOUNT; i++)
        pstEntry[i].set((void*)kDummyHandler, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
}

void IDTEntry::set(void* pvHandler, u16 wSelector, u8 bIST, u8 bFlags, u8 bType) {
    wLowerBaseAddress = u64(pvHandler) & 0xFFFF;
    wSegmentSelector = wSelector;
    this->bIST = bIST & 0x3;
    bTypeAndFlags = bType | bFlags;
    wMiddleBaseAddress = (u64(pvHandler) >> 16) & 0xFFFF;
    dwUpperBaseAddress = u64(pvHandler) >> 32;
    dwReserved = 0;
}

void kDummyHandler() {
    kPrintString(0, 0, "[INT] Dummy Interrupt Handler Execute");
    while(true);
}