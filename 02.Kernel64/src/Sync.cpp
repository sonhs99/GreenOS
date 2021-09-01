#include "Sync.hpp"
#include "Assembly.hpp"
#include "Utility.hpp"

bool kLockForSystemData() {
    return kSetInterruptFlag(false);
}

void kUnlockForSystemData(bool bInterruptFlag) {
    kSetInterruptFlag(bInterruptFlag);
}

void Mutex::lock() {
    if(!kTestAndSet(&bLockFlag, 0, 1)) {
        if(qwTaskID == kGetRunningTask()->qwID) {
            dwLockCount++;
            return;
        }
        while(!kTestAndSet(&bLockFlag, 0, 1)) kSchedule();
    }
    dwLockCount = 1;
    qwTaskID = kGetRunningTask()->qwID;
}

void Mutex::unlock() {
    if(!bLockFlag || (qwTaskID != kGetRunningTask()->qwID)) return;
    if(dwLockCount > 1) {
        dwLockCount--;
        return;
    }
    qwTaskID = TASK_INVALIDID;
    dwLockCount = 0;
    bLockFlag = false;
}