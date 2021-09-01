#pragma once

#include "Types.hpp"
#include "Task.hpp"

#pragma pack(push, 1)

class Mutex {
    volatile u64 qwTaskID;
    volatile u32 dwLockCount;
    
    volatile u8 bLockFlag;

    u8 vbPadding[3];

public:
    Mutex():bLockFlag(false), dwLockCount(0), qwTaskID(TASK_INVALIDID) {};
    void lock();
    void unlock();
};

#pragma pack(pop)

bool kLockForSystemData();
void kUnlockForSystemData(bool bInterruptFlag);