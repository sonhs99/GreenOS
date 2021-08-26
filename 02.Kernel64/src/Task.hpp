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

#pragma pack(push, 1)

struct Context {
    u64 vqRegister[TASK_REGISTERCOUNT];
    Context();
    Context(const Context & n);
};

struct Task : public ListNode {
    u64     qwFlags;
    Context stContext;
    
    void    *pvStackAddress;
    u64     qwStackSize;
    Task(u64 qwFlags, u64 qwEntryPointAddress, void* pvStackAddress, u64 qwStackSize);
    Task() {};
};

struct TaskPoolManager {
    Task* pstStartAddress;
    int iMaxCount;
    int iUseCount;
    int iAllocatedCount;
    TaskPoolManager();
};

struct Scheduler {
    Task *pstRunningTask;
    int iProcessorTime;
    List stReadyList;
};

#pragma pack(pop)

Task* kAllocateTask();
void kFreeTask(u64 qwID);
Task* kCreateTask(u64 qwFlags, u64 qwEntryPointAddress);

void kInitializeScheduler();
void kSetRunningTask(Task* pstTask);
Task* kGetRunningTask();
Task* kGetNextTaskToRun();
void kAddTaskToReadyList(Task* pstTask);
void kSchedule();
bool kScheduleInInterrupt();
bool kDecreaseProcessorTime();
bool kIsProcessorTimeExpired();
