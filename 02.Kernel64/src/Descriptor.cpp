#include "Descriptor.hpp"
#include "Utility.hpp"

void kInitializeGDTTableAndTSS() {
    GDTR* pstGDTR = (GDTR*) GDTR_STARTADDRESS;
    GDTEntry8* pstEntry = (GDTEntry8*)(GDTR_STARTADDRESS + sizeof(GDTR));
    TSSSegment* pstTSS = (TSSSegment*)((u64)pstEntry + GDT_TABLESIZE);

    pstGDTR->wLimit = GDT_TABLESIZE - 1;
    pstGDTR->qwBaseAddress = (u64)pstEntry;
    pstEntry[0].set(0, 0, 0, 0, 0);
    pstEntry[1].set(0, 0xFFFFF, GDT_FLAGS_UPPER_CODE, GDT_FLAGS_LOWER_KERNELCODE, GDT_TYPE_CODE);
    pstEntry[2].set(0, 0xFFFFF, GDT_FLAGS_UPPER_DATA, GDT_FLAGS_LOWER_KERNELDATA, GDT_TYPE_DATA);
    ((GDTEntry16*)&pstEntry[3])->set(u64(pstTSS), sizeof(TSS_SEGMENTSIZE) - 1, GDT_FLAGS_UPPER_TSS, GDT_FLAGS_LOWER_TSS, GDT_TYPE_TSS);

    pstTSS->init();
}

void GDTEntry8::set(u32 dwBaseAddress, u32 dwLimit, u8 bUpperFlags, u8 bLowerFlags, u8 bTypes) {

}

void GDTEntry16::set(u64 dwBaseAddress, u32 dwLimit, u8 bUpperFlags, u8 bLowerFlags, u8 bTypes) {
    
}