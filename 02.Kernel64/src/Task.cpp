#include "Task.hpp"
#include "Descriptor.hpp"
#include "Utility.hpp"
#include "Assembly.hpp"
#include "Console.hpp"

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
    gs_stTaskPoolManager.pstStartAddress->qwID = i;
    gs_stTaskPoolManager.iUseCount--;
}

Task* kCreateTask(u64 qwFlags, u64 qwEntryPointAddress) {
    Task *pstTask = kAllocateTask();
    if(pstTask == nullptr) return nullptr;

	void *pvStackAddress = (void *)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * GETTCBOFFSET(pstTask->qwID)));
    *pstTask = Task(pstTask->qwID, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);

    kAddTaskToReadyList(pstTask);
    return pstTask;
}

Task::Task(u64 qwID, u64 qwFlags, u64 qwEntryPointAddress, void* pvStackAddress, u64 qwStackSize) {
    stContext.vqRegister[TASK_RSPOFFSET] = u64(pvStackAddress) + qwStackSize;
    stContext.vqRegister[TASK_RBPOFFSET] = u64(pvStackAddress) + qwStackSize;

    stContext.vqRegister[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;
    stContext.vqRegister[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;
    stContext.vqRegister[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;
    stContext.vqRegister[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;
    stContext.vqRegister[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;
    stContext.vqRegister[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;

    stContext.vqRegister[TASK_RIPOFFSET] = qwEntryPointAddress;

    stContext.vqRegister[TASK_RFLAGOFFSET] |= 0x0200;

	this->qwID = qwID;
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
    gs_stScheduler.pstRunningTask = kAllocateTask();
    gs_stScheduler.pstRunningTask->qwFlags = TASK_FLAGS_HIGHEST;

    gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
    gs_stScheduler.qwProcessorLoad = 0;
}

void kSetRunningTask(Task *pstTask) {
    gs_stScheduler.pstRunningTask = pstTask;
}

Task* kGetRunningTask() {
    return gs_stScheduler.pstRunningTask;
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
    u8 bPriority = GETPRIORITY(pstTask->qwID);
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

bool ChangePriority(u64 qwTaskID, u8 bPriority) {
    if(bPriority > TASK_MAXREADYLISTCOUNT) return false;

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
    return true;
}

void kSchedule() {
    Task* pstRunningTask, *pstNextTask;
    bool bPreviousFlag;

    if(kGetReadyTaskCount() < 1) return;

    bPreviousFlag = kSetInterruptFlag(false);
    pstNextTask = kGetNextTaskToRun();
    if(pstNextTask == nullptr){
        kSetInterruptFlag(bPreviousFlag);
        return;
    }
    pstRunningTask = kGetRunningTask();
    kSetRunningTask(pstNextTask);

    if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime;

    if(pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK) {
        kAddTaskToReadyList(pstRunningTask);
        kSwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));
    }

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
    kSetInterruptFlag(bPreviousFlag);
}

bool kScheduleInInterrupt() {
    Task *pstRunningTask = kGetRunningTask(), *pstNextTask = kGetNextTaskToRun();
    char *pcContextAddress = (char*)(IST_STARTADDRESS + IST_SIZE - sizeof(Context));

    if(pstNextTask == nullptr) return false;

    kSetRunningTask(pstNextTask);

    if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE)
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
    
    if(pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK) kAddTaskToReadyList(pstRunningTask);
    else {
        kMemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(Context));
        kAddTaskToReadyList(pstRunningTask);
    }
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
    Task* pstTarget = gs_stScheduler.pstRunningTask;
    if(pstTarget->qwID == qwTaskID) {
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
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
            return false;
        }
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_ENDTASK);
        gs_stScheduler.stWaitList.AddListToTail(pstTarget);
    }
    return true;
}

void kExitTask(){
    kEndTask(gs_stScheduler.pstRunningTask->qwID);
}

int kGetReadyTaskCount() {
    int iTotalCount = 0;
    for(auto & stList: gs_stScheduler.vstReadyList)
        iTotalCount += stList.ItemCount();
    return iTotalCount;
}

int kGetTaskCount() {
    int iTotalCount = kGetReadyTaskCount();
    iTotalCount += gs_stScheduler.stWaitList.ItemCount();
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
                (qwCurrentSpendTickInIdleTask - qwLastMeasureTickCount) * 100 /
                (qwCurrentMeasureTickCount - qwLastMeasureTickCount);
        qwLastMeasureTickCount = qwCurrentMeasureTickCount;
        qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

        kHaltProcessorByLoad();
        if(gs_stScheduler.stWaitList.ItemCount() >= 0)
            while(true) {
                Task *pstTask = (Task*)gs_stScheduler.stWaitList.RemoveListFromHead();
                if(pstTask == nullptr) break;
                kPrintf("IDLE: Task ID[0x%q] is Completely ended.\n", pstTask->qwID);
                kFreeTask(pstTask->qwID);
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