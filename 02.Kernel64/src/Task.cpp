#include "Task.hpp"
#include "Descriptor.hpp"
#include "Utility.hpp"
#include "Assembly.hpp"

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

    for(i = 0; i < gs_stTaskPoolManager.iMaxCount; i++)
        if((gs_stTaskPoolManager.pstStartAddress[i].qwID >> 32) == 0) {
            pstEmptyTask = &(gs_stTaskPoolManager.pstStartAddress[i]);
            break;
        }

    pstEmptyTask->qwID = (u64(gs_stTaskPoolManager.iAllocatedCount) << 32) | i;
    gs_stTaskPoolManager.iUseCount++;
    gs_stTaskPoolManager.iAllocatedCount++;
    if(gs_stTaskPoolManager.iAllocatedCount == 0)
        gs_stTaskPoolManager.iAllocatedCount = 1;
    return pstEmptyTask;
}

void kFreeTask(u64 qwID) {
    int i = qwID & 0xFFFFFFFF;

    kMemSet(&(gs_stTaskPoolManager.pstStartAddress[i].stContext), 0, sizeof(Context));
    gs_stTaskPoolManager.pstStartAddress->qwID = i;
    gs_stTaskPoolManager.iUseCount--;
}

Task* kCreateTask(u64 qwFlags, u64 qwEntryPointAddress) {
    Task *pstTask = kAllocateTask();
    if(pstTask == nullptr) return nullptr;

    void *pvStackAddress = (void*)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * pstTask->qwID & 0xFFFFFFFF));
    *pstTask = Task(qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);

    kAddTaskToReadyList(pstTask);
    return pstTask;
}

Task::Task(u64 qwFlags, u64 qwEntryPointAddress, void* pvStackAddress, u64 qwStackSize) {
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
}

void kSetRunningTask(Task *pstTask) {
    gs_stScheduler.pstRunningTask = pstTask;
}

Task* kGetRunningTask() {
    return gs_stScheduler.pstRunningTask;
}

Task* kGetNextTaskToRun() {
    if(gs_stScheduler.stReadyList.ItemCount() == 0) return nullptr;
    return (Task*) gs_stScheduler.stReadyList.RemoveListFromHead();
}

void kAddTaskToReadyList(Task *pstTask) {
    gs_stScheduler.stReadyList.AddListToTail(pstTask);
}

void kSchedule() {
    Task* pstRunningTask, *pstNextTask;
    bool bPreviousFlag;

    if(gs_stScheduler.stReadyList.ItemCount() == 0) return;

    bPreviousFlag = kSetInterruptFlag(false);
    pstNextTask = kGetNextTaskToRun();
    if(pstNextTask == nullptr){
        kSetInterruptFlag(bPreviousFlag);
        return;
    }
    pstRunningTask = kGetRunningTask();
    kAddTaskToReadyList(pstRunningTask);

    kSetRunningTask(pstNextTask);
    kSwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
    kSetInterruptFlag(bPreviousFlag);
}

bool kScheduleInInterrupt() {
    Task *pstRunningTask = kGetRunningTask(), *pstNextTask = kGetNextTaskToRun();
    char *pcContextAddress = (char*)IST_STARTADDRESS + IST_SIZE - sizeof(Context);

    if(pstNextTask == nullptr) return false;

    kMemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(Context));
    kAddTaskToReadyList(pstRunningTask);

    kSetRunningTask(pstNextTask);
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