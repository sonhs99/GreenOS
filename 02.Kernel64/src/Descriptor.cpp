#include "Descriptor.hpp"
#include "Utility.hpp"
#include "IRQ.hpp"

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

    pstEntry[ 0].set((void*)kISRDivideError, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[ 1].set((void*)kISRDebug, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[ 2].set((void*)kISRNMI, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[ 3].set((void*)kISRBreakPoint, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[ 4].set((void*)kISROverflow, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[ 5].set((void*)kISRBoundRangeExceeded, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[ 6].set((void*)kISRInvalidOpcode, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[ 7].set((void*)kISRDeviceNotAvailable, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[ 8].set((void*)kISRDoubleFault, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[ 9].set((void*)kISRCoprocessorSegmentOverrun, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[10].set((void*)kISRInvalidTSS, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[11].set((void*)kISRSegmentNotPresent, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[12].set((void*)kISRStackSegmentFault, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[13].set((void*)kISRGeneralProtection, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[14].set((void*)kISRPageFault, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[15].set((void*)kISR15, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[16].set((void*)kISRFPUError, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[17].set((void*)kISRAlignmentCheck, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[18].set((void*)kISRMachineCheck, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[19].set((void*)kISRSIMDError, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[20].set((void*)kISRETCException, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);

    for(int i = 21; i < 32; i++)
        pstEntry[i].set((void*)kISRETCException, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);

    pstEntry[32].set((void*)kISRTimer, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[33].set((void*)kISRKeyboard, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[34].set((void*)kISRSlavePIC, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[35].set((void*)kISRSerial2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[36].set((void*)kISRSerial1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[37].set((void*)kISRParallel2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[38].set((void*)kISRFloppy, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[39].set((void*)kISRParallel1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[40].set((void*)kISRRTC, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[41].set((void*)kISRReserved, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[42].set((void*)kISRNotUsed1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[43].set((void*)kISRNotUsed2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[44].set((void*)kISRMouse, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[45].set((void*)kISRCoprocessor, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[46].set((void*)kISRHDD1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    pstEntry[47].set((void*)kISRHDD2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);

    for(int i = 48; i < IDT_MAXENTRYCOUNT; i++)
        pstEntry[i].set((void*)kISRETCInterrupt, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
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