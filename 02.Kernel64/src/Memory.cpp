#include "Memory.hpp"
#include "Utility.hpp"
#include "Task.hpp"
#include "Sync.hpp"
#include "Console.hpp"

static DynamicMemory gs_stDynamicMemory;

void kInitializeDynamicMemory() {
    u64 qwDynamicMemorySize = kCalculateDynamicMemorySize();
    int iMetaBlockCount = kCalculateMetaBlockCount(qwDynamicMemorySize), i;
    
    gs_stDynamicMemory.iBlockCountOfSmallestBlock = 
        (qwDynamicMemorySize / DYNAMICMEMORY_MIN_SIZE) - iMetaBlockCount;
    
    for(i = 0; (gs_stDynamicMemory.iBlockCountOfSmallestBlock >> i) > 0; i++);
    gs_stDynamicMemory.iMaxLevelCount = i;

    gs_stDynamicMemory.pbAllocatedBlockListIndex = (u8*) DYNAMICMEMORY_START_ADDRESS;
    for(i = 0; i < gs_stDynamicMemory.iBlockCountOfSmallestBlock; i++)
        gs_stDynamicMemory.pbAllocatedBlockListIndex[i] = 0xFF;

    gs_stDynamicMemory.pstBitmapOfLevel =  (Bitmap*)(DYNAMICMEMORY_START_ADDRESS +
        (sizeof(u8) * gs_stDynamicMemory.iBlockCountOfSmallestBlock));
    u8* pbCurrentBitmapPosition = ((u8*) gs_stDynamicMemory.pstBitmapOfLevel) +
        (sizeof(Bitmap) * gs_stDynamicMemory.iMaxLevelCount);

    for(int j = 0; j < gs_stDynamicMemory.iMaxLevelCount; j++) {
        gs_stDynamicMemory.pstBitmapOfLevel[j] = {
            .pbBitmap = pbCurrentBitmapPosition,
            .qwExistBitCount = 0
        };
        int iBlockCountOfLevel = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> j;
        for(i = 0; i < iBlockCountOfLevel / 8; i++) *(pbCurrentBitmapPosition++) = 0x00;
        if((i = (iBlockCountOfLevel % 8)) != 0) {
            *pbCurrentBitmapPosition = 0x00;
            if((i % 2) == 1) {
                *pbCurrentBitmapPosition |= (DYNAMICMEMORY_EXIST << (i - 1));
                gs_stDynamicMemory.pstBitmapOfLevel[j].qwExistBitCount = 1;
            }
            pbCurrentBitmapPosition++;
        }
    }

    gs_stDynamicMemory.qwStartAddress = DYNAMICMEMORY_START_ADDRESS + iMetaBlockCount * DYNAMICMEMORY_MIN_SIZE;
    gs_stDynamicMemory.qwEndAddress = kCalculateDynamicMemorySize() + DYNAMICMEMORY_START_ADDRESS;
    gs_stDynamicMemory.qwUsedSize = 0;
}

static u64 kCalculateDynamicMemorySize() {
    u64 qwRAMSize = (kGetTotalRAMSize() * 1024 * 1024);
    qwRAMSize = qwRAMSize > u64(3) * 1024 * 1024 * 1024 ?
        u64(3) * 1024 * 1024 * 1024 : qwRAMSize;
    return qwRAMSize - DYNAMICMEMORY_START_ADDRESS; 
}

static int kCalculateMetaBlockCount(u64 qwDynamicRAMSize) {
    long lBlockCountOfSmallestBlock  = qwDynamicRAMSize / DYNAMICMEMORY_MIN_SIZE;
    u32 dwSizeOfAllocatedBlockListIndex = lBlockCountOfSmallestBlock * sizeof(u8);
    u32 dwSizeOfBitmap = 0;
    for(int i = 0; (lBlockCountOfSmallestBlock >> i) > 0; i++) {
        dwSizeOfBitmap += sizeof(Bitmap);
        dwSizeOfBitmap += ((lBlockCountOfSmallestBlock >> i) + 7) / 8;
    }
    return (dwSizeOfAllocatedBlockListIndex + dwSizeOfBitmap + DYNAMICMEMORY_MIN_SIZE - 1) / DYNAMICMEMORY_MIN_SIZE;
}

void* kAllocateMemory(u64 qwSize) {
    u64 qwAlignedSize = kGetBuddyBlockSize(qwSize);
    if(qwAlignedSize == 0) return nullptr;
    if(gs_stDynamicMemory.qwStartAddress + gs_stDynamicMemory.qwUsedSize + qwAlignedSize > gs_stDynamicMemory.qwEndAddress)
        return nullptr;
    
    long lOffset = kAllocationBuddyBlock(qwAlignedSize);
    if(lOffset == -1) return nullptr;

    int iIndexOfBlockList = kGetBlockListIndexOfMatchSize(qwAlignedSize);
    u64 qwRelativeAddress = qwAlignedSize * lOffset;
    int iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;
    
    gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset] = u8(iIndexOfBlockList);
    gs_stDynamicMemory.qwUsedSize += qwAlignedSize;
    return (void*) (qwRelativeAddress + gs_stDynamicMemory.qwStartAddress);
}

static u64 kGetBuddyBlockSize(u64 qwSize) {
    for(long i = 0; i < gs_stDynamicMemory.iMaxLevelCount; i++) 
        if(qwSize <= (DYNAMICMEMORY_MIN_SIZE << i))
            return (DYNAMICMEMORY_MIN_SIZE << i);
    return 0;
}

static int kAllocationBuddyBlock(u64 qwAlignedSize) {
    int iBlockListIndex = kGetBlockListIndexOfMatchSize(qwAlignedSize), iFreeOffset, i;
    if(iBlockListIndex == -1) return -1;

    bool bPreviousInterruptFlag = kLockForSystemData();
    for(i = iBlockListIndex; i < gs_stDynamicMemory.iMaxLevelCount; i++)
        if((iFreeOffset = kFindFreeBlockInBitmap(i)) != -1) break;

    if(iFreeOffset == -1){
        kUnlockForSystemData(bPreviousInterruptFlag);
        return -1;
    }

    kSetFlagInBitmap(i, iFreeOffset, DYNAMICMEMORY_EMPTY);
    if(i > iBlockListIndex)
        for(i = i - 1; i >= iBlockListIndex; i--) {
            kSetFlagInBitmap(i, iFreeOffset * 2, DYNAMICMEMORY_EMPTY);
            kSetFlagInBitmap(i, iFreeOffset * 2 + 1, DYNAMICMEMORY_EXIST);
            iFreeOffset *= 2;
        }
    kUnlockForSystemData(bPreviousInterruptFlag);
    return iFreeOffset;
}

static int kGetBlockListIndexOfMatchSize(u64 qwAlignedSize) {
    for(int i = 0; i < gs_stDynamicMemory.iMaxLevelCount; i++)
        if(qwAlignedSize <= (DYNAMICMEMORY_MIN_SIZE << i))
            return i;
    return -1;
}

static int kFindFreeBlockInBitmap(int iBlockListIndex) {
    if(gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].qwExistBitCount == 0)
        return -1;

    int iMaxCount = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> iBlockListIndex;
    u8 *pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].pbBitmap;
    for(int i = 0; i < iMaxCount;) {
        if(((iMaxCount - i) / 64) > 0) {
            u64 *pqwBitmap = (u64*) &(pbBitmap[i / 8]);
            if(*pqwBitmap == 0) {
                i += 64;
                continue;
            }
        }
        if((pbBitmap[i / 8] & (DYNAMICMEMORY_EXIST << ( i % 8 ))) != 0) return i;
        i++;
    }
    return -1;
}

static void kSetFlagInBitmap(int iBlockListIndex, int iOffset, u8 bFlag) {
    u8 *pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].pbBitmap;
    if(bFlag == DYNAMICMEMORY_EXIST){
        if((pbBitmap[iOffset / 8] & (0x01 << (iOffset % 8))) == 0)
            gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].qwExistBitCount++;
        pbBitmap[iOffset / 8] |= (0x01 << (iOffset % 8));
    } else {
        if((pbBitmap[iOffset / 8] & (0x01 << (iOffset % 8))) != 0)
            gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].qwExistBitCount--;
        pbBitmap[iOffset / 8] &= ~(0x01 << (iOffset % 8));
    }
}

bool kFreeMemory(void *pvAddress) {
    if(pvAddress == nullptr) return false;
    u64 qwRelativeAddress = u64(pvAddress) - gs_stDynamicMemory.qwStartAddress;
    int iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;
    if(gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset] == 0xFF)
        return false;

    int iBlockListIndex = int(gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset]);
    gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset] = 0xFF;
    u64 qwBlockSize = DYNAMICMEMORY_MIN_SIZE << iBlockListIndex;

    int iBitmapOffset = qwRelativeAddress / qwBlockSize;
    if(kFreeBuddyBlock(iBlockListIndex, iBitmapOffset)) {
        gs_stDynamicMemory.qwUsedSize -= qwBlockSize;
        return true;
    }
    return false;
}

static bool kFreeBuddyBlock(int iBlockListIndex, int iBlockOffset) {
    int iBuddyBlockOffset;
    bool bPreviousInterruptFlag = kLockForSystemData();
    for(int i = iBlockListIndex; i < gs_stDynamicMemory.iMaxLevelCount; i++) {
        kSetFlagInBitmap(i, iBlockOffset, DYNAMICMEMORY_EXIST);
        if((iBlockOffset % 2) == 0) iBuddyBlockOffset = iBlockOffset + 1;
        else iBuddyBlockOffset = iBlockOffset - 1;
        
        if(kGetFlagInBitmap(i, iBuddyBlockOffset) == DYNAMICMEMORY_EMPTY)
            break;

        kSetFlagInBitmap(i, iBuddyBlockOffset, DYNAMICMEMORY_EMPTY);
        kSetFlagInBitmap(i, iBlockOffset, DYNAMICMEMORY_EMPTY);
        iBlockOffset = iBlockOffset / 2;
    }
    kUnlockForSystemData(bPreviousInterruptFlag);
    return true;
}

static u8 kGetFlagInBitmap(int iBlockListIndex, int iOffset) {
    u8 *pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].pbBitmap;
    if((pbBitmap[iOffset / 8] & (0x01 << (iOffset % 8))) != 0)
        return DYNAMICMEMORY_EXIST;
    return DYNAMICMEMORY_EMPTY;
}

void kGetDynamicMemoryInformation(
    u64 &pqwDynamicMemoryStartAddress,
    u64 &pqwDynamicMemoryTotalSize,
    u64 &pqwMetaDataSize,
    u64 &pqwUsedMemorySize) {
    pqwDynamicMemoryStartAddress = DYNAMICMEMORY_START_ADDRESS;
    pqwDynamicMemoryTotalSize = kCalculateDynamicMemorySize();
    pqwMetaDataSize = kCalculateMetaBlockCount(pqwDynamicMemoryTotalSize) * DYNAMICMEMORY_MIN_SIZE;
    pqwUsedMemorySize = gs_stDynamicMemory.qwUsedSize;
}

DynamicMemory& kGetDynamicMemoryManager() {
    return gs_stDynamicMemory;
}