#pragma once

#include "Types.hpp"

#define DYNAMICMEMORY_START_ADDRESS (( TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * TASK_MAXCOUNT ) + 0xFFFFF ) & 0xFFFFFFFFFFF00000 )
#define DYNAMICMEMORY_MIN_SIZE      ( 1 * 1024 )

#define DYNAMICMEMORY_EXIST         0x01
#define DYNAMICMEMORY_EMPTY         0x00

struct Bitmap {
    u8  *pbBitmap;
    u64 qwExistBitCount;
};

struct DynamicMemory {
    int iMaxLevelCount;
    int iBlockCountOfSmallestBlock;
    u64 qwUsedSize;

    u64 qwStartAddress;
    u64 qwEndAddress;

    u8  *pbAllocatedBlockListIndex;
    Bitmap *pstBitmapOfLevel;
};

void kInitializeDynamicMemory();
void* kAllocateMemory(u64 qwSize);
bool kFreeMemory(void* pvAddress);
void kGetDynamicMemoryInformation(u64 &pqwDynamicMemoryAddress, u64 &pqwDynamicMemoryTotalSize, u64 &pqwMetaDataSize, u64 &pqwUsedMemorySize);
DynamicMemory& kGetDynamicMemoryManager();

static u64 kCalculateDynamicMemorySize();
static int kCalculateMetaBlockCount(u64 qwDyanmicRAMSize);
static int kAllocationBuddyBlock(u64 qwAllignedSize);
static u64 kGetBuddyBlockSize(u64 qwSize);
static int kGetBlockListIndexOfMatchSize(u64 qwAllignedSize);
static int kFindFreeBlockInBitmap(int iBlockListIndex);
static void kSetFlagInBitmap(int iBlockListIndex, int iBlockOffset, u8 bFlag);
static bool kFreeBuddyBlock(int iBlockListIndex, int iBlockOffset);
static u8 kGetFlagInBitmap(int iBlockListIndex, int iOffset);