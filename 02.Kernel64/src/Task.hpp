#pragma once

#include "Types.hpp"
#include "List.hpp"

#define TASK_REGISTERCOUNT      ( 5 + 19 )
#define TASK_REGISTERSIZE       8

#define TASK_GSOFFSET           0
#define TASK_FSOFFSET           1
#define TASK_ESOFFSET           2
#define TASK_DSOFFSET           3
#define TASK_R15OFFSET          4
#define TASK_R14OFFSET          5
#define TASK_R13OFFSET          6
#define TASK_R12OFFSET          7
#define TASK_R11OFFSET          8
#define TASK_R10OFFSET          9
#define TASK_R9OFFSET           10
#define TASK_R8OFFSET           11
#define TASK_RSIOFFSET          12
#define TASK_RDIOFFSET          13
#define TASK_RDXOFFSET          14
#define TASK_RCXOFFSET          15
#define TASK_RBXOFFSET          16
#define TASK_RAXOFFSET          17
#define TASK_RBPOFFSET          18
#define TASK_RIPOFFSET          19 
#define TASK_CSOFFSET           20
#define TASK_RFLAGOFFSET        21
#define TASK_RSPOFFSET          22
#define TASK_SSOFFSET           23

#define TASK_TCBPOOLADDRESS     0x800000
#define TASK_MAXCOUNT           1024

#define TASK_STACKPOOLADDRESS   (TASK_TCBPOOLADDRESS + sizeof(Task) * TASK_MAXCOUNT)
#define TASK_STACKSIZE          8192

#define TASK_INVALIDID          0xFFFFFFFFFFFFFFFF
#define TASK_PROCESSORTIME      5

#define TASK_MAXREADYLISTCOUNT  5

#define TASK_FLAGS_HIGHEST      0
#define TASK_FLAGS_HIGH         1
#define TASK_FLAGS_MEDIUM       2
#define TASK_FLAGS_LOW          3
#define TASK_FLAGS_LOWEST       4
#define TASK_FLAGS_WAIT         0xFF

#define TASK_FLAGS_ENDTASK      0x8000000000000000
#define TASK_FLAGS_SYSTEM       0x4000000000000000
#define TASK_FLAGS_PROCESS      0x2000000000000000
#define TASK_FLAGS_THREAD       0x1000000000000000
#define TASK_FLAGS_IDLE         0x0800000000000000

#define GETPRIORITY(X)              ( (X) & 0xFF )
#define SETPRIORITY(X, priority)    ( (X) = ( (X) & 0xFFFFFFFFFFFFFF00 ) )
#define GETTCBOFFSET(X)             ( (X) & 0xFFFFFFFF )
#define GETTCBFROMTHREADLINK(X)     (Task*) (u64(X) - offsetof(Task, stThreadLink))

#pragma pack(push, 1)

struct Context {
    u64 vqRegister[TASK_REGISTERCOUNT];
    Context();
    Context(const Context & n);
};

struct Task : public ListNode {
    u64     qwFlags;

    void   *pvMemoryAddress;
    u64     qwMemorySize;

    ListNode stThreadLink;
    List    stChildThreadList;

    u64     qwParentProcessID;

    Context stContext;
    
    void    *pvStackAddress;
    u64     qwStackSize;
    Task(u64 qwID, u64 qwFlags, u64 qwEntryPointAddress, void* pvStackAddress, u64 qwStackSize) {
        set(qwFlags, qwEntryPointAddress, pvStackAddress, qwStackSize);
        this->qwID = qwID;
    }
    void set(u64 qwFlags, u64 qwEntryPointAddress, void* pvStackAddress, u64 qwStackSize);
};

struct TaskPoolManager {
    Task* pstStartAddress;
    int iMaxCount;
    int iUseCount;
    int iAllocatedCount;
};

struct Scheduler {
    Task *pstRunningTask;
    int iProcessorTime;

    List vstReadyList[TASK_MAXREADYLISTCOUNT];
    List stWaitList;
    int viExecuteCount[TASK_MAXREADYLISTCOUNT];
    u64 qwProcessorLoad;
    u64 qwSpendProcessorTimeInIdleTask;
};

#pragma pack(pop)

Task* kAllocateTask();
void kFreeTask(u64 qwID);
Task* kCreateTask(u64 qwFlags, void* pvMemoryAddress, u64 qwMemorySize, u64 qwEntryPointAddress);

void kInitializeScheduler();
void kSetRunningTask(Task* pstTask);
Task* kGetRunningTask();
Task* kGetNextTaskToRun();
bool kAddTaskToReadyList(Task* pstTask);
void kSchedule();
bool kScheduleInInterrupt();
void kDecreaseProcessorTime();
bool kIsProcessorTimeExpired();

Task* kRemoveTaskFromReadyList(u64 qwTaskID);
bool kChangePriority(u64 qwID, u8 bPriority);
bool kEndTask(u64 qwTaskID);
void kExitTask();
int  kGetReadyTaskCount();
int  kGetTaskCount();
Task* kGetTaskInTCBPool(int iOffset);
bool kIsTaskExist(u64 qwID);
u64  kGetProcessorLoad();

void kIdleTask();
void kHaltProcessorByLoad();
Task* kGetProcessByThread(Task* pstThread);
