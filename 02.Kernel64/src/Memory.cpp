#include "Memory.hpp"
#include "Utility.hpp"
#include "Task.hpp"

static DynamicMemory gs_stDynamicMemory;

void kInitializeDynamicMemory() {
    u64 qwDynamicMemorySize = kCalculatedDynamicMemorySize();
    int iMetaBlockCount = kCalculatedMetaBlockCount(qwDynamicMemorySize), i;
    
    gs_stDynamicMemory.iBlockCountOfSmallestBlock = 
        (qwDynamicMemorySize / DYNAMICMEMORY_MIN_SIZE) - iMetaBlockCount;
    
    for(i = 0; (gs_stDynamicMemory.iBlockCountOfSmallestBlock >> i) > 0; i++);
    gs_stDynamicMemory.iMaxLevelCount = i;

    gs_stDynamicMemory.pbAllocatedBlockListIndex = (u8*) DYNAMICMEMORY_START_ADDRESS;
    for(i = 0; i < gs_stDynamicMemory.iBlockCountOfSmallestBlock; i++)
        gs_stDynamicMemory.pbAllocatedBlockListIndex[i] = 0xFF;

    gs_stDynamicMemory.pstBitmapOfLevel = (Bitmap*)(DYNAMICMEMORY_START_ADDRESS + (sizeof(u8) * gs_stDynamicMemory.iMaxLevelCount));
    u8* pbCurrentBitmapPosition = ((u8*) gs_stDynamicMemory.pstBitmapOfLevel) + (sizeof(Bitmap) * gs_stDynamicMemory.iBlockCountOfSmallestBlock);
    for(int j = 0; j < gs_stDynamicMemory.iMaxLevelCount; j++) {
        gs_stDynamicMemory.pstBitmapOfLevel[j] = {
            .pbBitmap = pbCurrentBitmapPosition,
            .qwExistBitCount = 0
        };
        int iBlockCountLevel = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> j;
        for(i = 0; i < iBlockCountLevel / 8; i++) *(pbCurrentBitmapPosition++) = 0;
        if((i = iBlockCountLevel % 8) != 0) {
            *pbCurrentBitmapPosition = 0;
            if((i % 2) == 1) {
                *(pbCurrentBitmapPosition++) |= (DYNAMICMEMORY_EXIST << (i - 1));
                gs_stDynamicMemory.pstBitmapOfLevel[j].qwExistBitCount = 1;
            }
        }
    }

    gs_stDynamicMemory.qwStartAddress = DYNAMICMEMORY_START_ADDRESS + iMetaBlockCount * DYNAMICMEMORY_MIN_SIZE;
    gs_stDynamicMemory.qwEndAddress = kCalculatedDynamicMemorySize() + DYNAMICMEMORY_START_ADDRESS;
    gs_stDynamicMemory.qwUsedSize = 0;
}

static u64 kCalculatedDynamicMemorySize() {
    u64 qwRAMSize = (kGetTotalRAMSize() * 1024 * 1024);
    qwRAMSize = qwRAMSize > u64(3) * 1024 * 1024 * 1024 ?
        u64(3) * 1024 * 1024 * 1024 : qwRAMSize;
    return qwRAMSize - DYNAMICMEMORY_START_ADDRESS; 
}

static int kCalculatedMetaBlockCount(u64 qwDynamicRAMSize) {
    long lBlockCountOfSmallestBlock  = qwDynamicRAMSize / DYNAMICMEMORY_MIN_SIZE;
    u32 dwSizeOfAllocatedBlockListIndex = lBlockCountOfSmallestBlock * sizeof(u8);
    u32 dwSizeOfBitmap = 0;
    for(int i = 0; (lBlockCountOfSmallestBlock >> i) > 0; i++) {
        dwSizeOfBitmap += sizeof(Bitmap);
        dwSizeOfBitmap += ((lBlockCountOfSmallestBlock >> i) + 7) / 8;
    }
    return (dwSizeOfAllocatedBlockListIndex + dwSizeOfBitmap + DYNAMICMEMORY_MIN_SIZE - 1) / DYNAMICMEMORY_MIN_SIZE;
}

void* kAllocatedMemory(u64 qwSize) {
    
}