#include "Task.hpp"
#include "Descriptor.hpp"
#include "Utility.hpp"
#include "Assembly.hpp"
#include "Console.hpp"
#include "Sync.hpp"

static Scheduler gs_stScheduler;
static TaskPoolManager gs_stTaskPoolManager;

void kInitializeTaskPool(TaskPoolManager& pstManager) {
    kMemSet(&pstManager, 0, sizeof(TaskPoolManager));
    pstManager.pstStartAddress = (Task*)TASK_TCBPOOLADDRESS;
    kMemSet(pstManager.pstStartAddress, 0, sizeof(Task) * TASK_MAXCOUNT);
    for(int i = 0; i < TASK_MAXCOUNT; i++)
        pstManager.pstStartAddress[i].qwID = i;
    
    pstManager.iMaxCount = TASK_MAXCOUNT;
    pstManager.iAllocatedCount = 1;
}

Task* kAllocateTask() {
    Task* pstEmptyTask;
    int i;

    if(gs_stTaskPoolManager.iUseCount == gs_stTaskPoolManager.iMaxCount) return nullptr;

    for(i = 0; i < gs_stTaskPoolManager.iMaxCount; i++) {
		if((gs_stTaskPoolManager.pstStartAddress[i].qwID >> 32) == 0) {
            pstEmptyTask = &(gs_stTaskPoolManager.pstStartAddress[i]);
            break;
        }
	}

	pstEmptyTask->qwID = (u64(gs_stTaskPoolManager.iAllocatedCount) << 32) | i;
	gs_stTaskPoolManager.iUseCount++;
	gs_stTaskPoolManager.iAllocatedCount++;
    if(gs_stTaskPoolManager.iAllocatedCount == 0)
        gs_stTaskPoolManager.iAllocatedCount = 1;
	return pstEmptyTask;
}

void kFreeTask(u64 qwID) {
    int i = GETTCBOFFSET(qwID);

    kMemSet(&(gs_stTaskPoolManager.pstStartAddress[i].stContext), 0, sizeof(Context));
    gs_stTaskPoolManager.pstStartAddress[i].qwID = i;
    gs_stTaskPoolManager.iUseCount--;
}

Task* kCreateTask(u64 qwFlags, void* pvMemoryAddress, u64 qwMemorySize, u64 qwEntryPointAddress) {
    bool bPreviousFlag = kLockForSystemData();
    Task *pstTask = kAllocateTask();
    if(pstTask == nullptr) {
        kUnlockForSystemData(bPreviousFlag);
        return nullptr;
    }
    Task *pstProcess = kGetProcessByThread(kGetRunningTask());
    if(pstProcess == nullptr){
        kFreeTask(pstTask->qwID);
        kUnlockForSystemData(bPreviousFlag);
        return nullptr;
    }
    if(qwFlags & TASK_FLAGS_THREAD) {
        pstTask->qwParentProcessID = pstProcess->qwID;
        pstTask->pvMemoryAddress = pstProcess->pvMemoryAddress;
        pstTask->qwMemorySize = pstProcess->qwMemorySize;
        pstProcess->stChildThreadList.AddListToTail(pstTask);
    } else {
        pstTask->qwParentProcessID = pstProcess->qwID;
        pstTask->pvMemoryAddress = pvMemoryAddress;
        pstTask->qwMemorySize = qwMemorySize;
    }
    pstTask->stThreadLink.qwID = pstTask->qwID;
    kUnlockForSystemData(bPreviousFlag);

	void *pvStackAddress = (void *)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * GETTCBOFFSET(pstTask->qwID)));
    pstTask->set(qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);
    pstTask->stChildThreadList = List();

    bPreviousFlag = kLockForSystemData();
    kAddTaskToReadyList(pstTask);
    kUnlockForSystemData(bPreviousFlag);

    return pstTask;
}

void Task::set(u64 qwFlags, u64 qwEntryPointAddress, void* pvStackAddress, u64 qwStackSize) {
    stContext.vqRegister[TASK_RSPOFFSET] = u64(pvStackAddress) + qwStackSize - 8;
    stContext.vqRegister[TASK_RBPOFFSET] = u64(pvStackAddress) + qwStackSize - 8;

    *(u64*)(u64(pvStackAddress + qwStackSize - 8)) = u64(kExitTask);

    stContext.vqRegister[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;
    stContext.vqRegister[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;
    stContext.vqRegister[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;
    stContext.vqRegister[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;
    stContext.vqRegister[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;
    stContext.vqRegister[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;

    stContext.vqRegister[TASK_RIPOFFSET] = qwEntryPointAddress;

    stContext.vqRegister[TASK_RFLAGOFFSET] |= 0x0200;

    this->pvStackAddress = pvStackAddress;
    this->qwStackSize = qwStackSize;
    this->qwFlags = qwFlags;
}

Context::Context() {
    kMemSet(vqRegister, 0, sizeof(vqRegister));
}

Context::Context(const Context & n) {
    kMemCpy(vqRegister, n.vqRegister, sizeof(vqRegister));
}

void kInitializeScheduler() {
    kInitializeTaskPool(gs_stTaskPoolManager);
    for(int i = 0; i < TASK_MAXREADYLISTCOUNT; i++) gs_stScheduler.viExecuteCount[i] = 0;

    Task *pstTask = kAllocateTask();
    gs_stScheduler.pstRunningTask = pstTask;
    pstTask->qwFlags = TASK_FLAGS_HIGHEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM;
    pstTask->qwParentProcessID = pstTask->qwID;
    pstTask->pvMemoryAddress = (void*) 0x100000;
    pstTask->qwMemorySize = 0x500000;
    pstTask->qwStackSize = 0x100000;

    gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
    gs_stScheduler.qwProcessorLoad = 0;
}

Task* kGetProcessByThread(Task *pstThread) {
    if(pstThread->qwFlags & TASK_FLAGS_PROCESS) return pstThread;
    Task* pstProcess = kGetTaskInTCBPool(GETTCBOFFSET(pstThread->qwParentProcessID));
    if((pstProcess == nullptr) || (pstProcess->qwID != pstProcess->qwParentProcessID))
        return nullptr;
    return pstProcess;
}

void kSetRunningTask(Task *pstTask) {
    bool bPreviousFlag = kLockForSystemData();
    gs_stScheduler.pstRunningTask = pstTask;
    kUnlockForSystemData(bPreviousFlag);
}

Task* kGetRunningTask() {
    bool bPreviousFlag = kLockForSystemData();
    Task* pstTask = gs_stScheduler.pstRunningTask;
    kUnlockForSystemData(bPreviousFlag);
    return pstTask;
}

Task* kGetNextTaskToRun() {
    Task* pstTarget = nullptr;
    for(int j = 0; j < 2; j++) {
        for(int i = 0; i < TASK_MAXREADYLISTCOUNT; i++) {
            int iTaskCount = gs_stScheduler.vstReadyList[i].ItemCount();
            if(gs_stScheduler.viExecuteCount[i] < iTaskCount) {
                pstTarget = (Task*) gs_stScheduler.vstReadyList[i].RemoveListFromHead();
                gs_stScheduler.viExecuteCount[i]++;
                break;
            } else gs_stScheduler.viExecuteCount[i] = 0;
        }
        if(pstTarget != nullptr) break;
    }
    return pstTarget;
}

bool kAddTaskToReadyList(Task *pstTask) {
    u8 bPriority = GETPRIORITY(pstTask->qwFlags);
    if(bPriority >= TASK_MAXREADYLISTCOUNT) return false;
    gs_stScheduler.vstReadyList[bPriority].AddListToTail(pstTask);
    return true;
}

Task* kRemoveTaskFromReadyList(u64 qwTaskID) {
    if(GETTCBOFFSET(qwTaskID) >= TASK_MAXCOUNT) return nullptr;

    Task* pstTarget = &(gs_stTaskPoolManager.pstStartAddress[GETTCBOFFSET(qwTaskID)]);
    if(pstTarget->qwID != qwTaskID) return nullptr;

    u8 bPriority = GETPRIORITY(pstTarget->qwFlags);
    pstTarget = (Task*)gs_stScheduler.vstReadyList[bPriority].Remove(qwTaskID);
    return pstTarget;
}

bool kChangePriority(u64 qwTaskID, u8 bPriority) {
    if(bPriority > TASK_MAXREADYLISTCOUNT) return false;

    bool bPreviousFlag = kLockForSystemData();
    Task* pstTarget = gs_stScheduler.pstRunningTask;
    if(pstTarget->qwID == qwTaskID) SETPRIORITY(pstTarget->qwFlags, bPriority);
    else {
        pstTarget = kRemoveTaskFromReadyList(qwTaskID);
        if(pstTarget == nullptr) {
            pstTarget = kGetTaskInTCBPool(GETTCBOFFSET(qwTaskID));
            if(pstTarget != nullptr) SETPRIORITY(pstTarget->qwFlags, bPriority);
        } else {
            SETPRIORITY(pstTarget->qwFlags, bPriority);
            kAddTaskToReadyList(pstTarget);
        }
    }
    kUnlockForSystemData(bPreviousFlag);
    return true;
}

void kSchedule() {
    if(kGetReadyTaskCount() < 1) return;

    bool bPreviousFlag = kLockForSystemData();
    Task* pstNextTask = kGetNextTaskToRun();
    if(pstNextTask == nullptr){
        kSetInterruptFlag(bPreviousFlag);
        return;
    }
    Task* pstRunningTask = kGetRunningTask();
    kSetRunningTask(pstNextTask);

    if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime;

    if(pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK) {
        gs_stScheduler.stWaitList.AddListToTail(pstRunningTask);
        kSwitchContext(nullptr, &(pstNextTask->stContext));
    } else {
        kAddTaskToReadyList(pstRunningTask);
        kSwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));
    }

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
    kUnlockForSystemData(bPreviousFlag);
}

bool kScheduleInInterrupt() {
    bool bPreviousFlag = kLockForSystemData();
    Task *pstRunningTask = kGetRunningTask(), *pstNextTask = kGetNextTaskToRun();
    char *pcContextAddress = (char*)(IST_STARTADDRESS + IST_SIZE - sizeof(Context));

    if(pstNextTask == nullptr) {
        kUnlockForSystemData(bPreviousFlag);
        return false;
    }
    kSetRunningTask(pstNextTask);

    if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;

    if(pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK) gs_stScheduler.stWaitList.AddListToTail(pstRunningTask);
    else {
        kMemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(Context));
        kAddTaskToReadyList(pstRunningTask);
    }
    kUnlockForSystemData(bPreviousFlag);
    kMemCpy(pcContextAddress, &(pstNextTask->stContext), sizeof(Context));

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
    return true;
}

void kDecreaseProcessorTime() {
    if(gs_stScheduler.iProcessorTime > 0) gs_stScheduler.iProcessorTime--;
}

bool kIsProcessorTimeExpired() {
    return gs_stScheduler.iProcessorTime <= 0;
}

bool kEndTask(u64 qwTaskID) {
    bool bPreviousFlag = kLockForSystemData();
    Task* pstTarget = gs_stScheduler.pstRunningTask;
    if(pstTarget->qwID == qwTaskID) {
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
        kUnlockForSystemData(bPreviousFlag);
        kSchedule();
        while(true);
    } else {
        pstTarget = kRemoveTaskFromReadyList(qwTaskID);
        if(pstTarget == nullptr) {
            pstTarget = kGetTaskInTCBPool(GETTCBOFFSET(qwTaskID));
            if(pstTarget != nullptr) {
                pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
                SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
            }
            kUnlockForSystemData(bPreviousFlag);
            return false;
        }
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_ENDTASK);
        gs_stScheduler.stWaitList.AddListToTail(pstTarget);
    }
    kUnlockForSystemData(bPreviousFlag);
    return true;
}

void kExitTask(){
    kEndTask(gs_stScheduler.pstRunningTask->qwID);
}

int kGetReadyTaskCount() {
    int iTotalCount = 0;
    bool bPreviousFlag = kLockForSystemData();
    for(auto & stList: gs_stScheduler.vstReadyList)
        iTotalCount += stList.ItemCount();
    kUnlockForSystemData(bPreviousFlag);
    return iTotalCount;
}

int kGetTaskCount() {
    int iTotalCount = kGetReadyTaskCount();
    bool bPreviousFlag = kLockForSystemData();
    iTotalCount += gs_stScheduler.stWaitList.ItemCount();
    kUnlockForSystemData(bPreviousFlag);
    return iTotalCount;
}

Task* kGetTaskInTCBPool(int iOffset) {
    if((iOffset < -1) || (iOffset > TASK_MAXCOUNT)) return nullptr;
    return &(gs_stTaskPoolManager.pstStartAddress[iOffset]);
}

bool kIsTaskExist(u64 qwID) {
    Task *pstTask = kGetTaskInTCBPool(GETTCBOFFSET(qwID));
    if((pstTask == nullptr) || (pstTask->qwID != qwID)) return false;
    return true;
}

u64 kGetProcessorLoad() {
    return gs_stScheduler.qwProcessorLoad;
}

void kIdleTask() {
    u64 qwLastSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
    u64 qwLastMeasureTickCount = kGetTickCount();

    while(true) {
        u64 qwCurrentMeasureTickCount = kGetTickCount();
        u64 qwCurrentSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;

        if(qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0) gs_stScheduler.qwProcessorLoad = 0;
        else
            gs_stScheduler.qwProcessorLoad = 100 -
                (qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask) * 100 /
                (qwCurrentMeasureTickCount - qwLastMeasureTickCount);
        qwLastMeasureTickCount = qwCurrentMeasureTickCount;
        qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

        kHaltProcessorByLoad();
        if(gs_stScheduler.stWaitList.ItemCount() >= 0)
            while(true) {
                bool bPreviousFlag = kLockForSystemData();
                Task *pstTask = (Task*)gs_stScheduler.stWaitList.RemoveListFromHead();
                if(pstTask == nullptr) {
                    kUnlockForSystemData(bPreviousFlag);
                    break;
                }
                if(pstTask->qwFlags & TASK_FLAGS_PROCESS) {
                    int iCount = pstTask->stChildThreadList.ItemCount();
                    for(int i = 0; i < iCount; i++) {
                        ListNode *pstThreadLink = pstTask->stChildThreadList.RemoveListFromHead();
                        if(pstThreadLink == nullptr) break;
                        Task *pstThread = GETTCBFROMTHREADLINK(pstThreadLink);
                        pstTask->stChildThreadList.AddListToTail(&(pstThread->stThreadLink));
                        kEndTask(pstThread->qwID);
                    }
                    if(pstTask->stChildThreadList.ItemCount() > 0) {
                        gs_stScheduler.stWaitList.AddListToTail(pstTask);
                        kUnlockForSystemData(bPreviousFlag);
                        continue;
                    } else { /* To Do */ }
                }
                else if(pstTask->qwFlags & TASK_FLAGS_THREAD) {
                    Task *pstProcess = kGetProcessByThread(pstTask);
                    if(pstProcess != nullptr)
                        pstProcess->stChildThreadList.Remove(pstTask->qwID);
                }
                u64 qwTaskID = pstTask->qwID;
                kFreeTask(qwTaskID);
                kUnlockForSystemData(bPreviousFlag);
                kPrintf("IDLE: Task ID[0x%q] is Completely ended.\n", qwTaskID);
            }

        kSchedule();
    }
}

void kHaltProcessorByLoad() {
    if(gs_stScheduler.qwProcessorLoad < 40) {
        kHlt();
        kHlt();
        kHlt();
    } else if(gs_stScheduler.qwProcessorLoad < 80) {
        kHlt();
        kHlt();
    } else if(gs_stScheduler.qwProcessorLoad < 95) {
        kHlt();
    }
}