#include "Task.hpp"
#include "Descriptor.hpp"
#include "Utility.hpp"

Task::Task(u64 qwID, u64 qwEntryPointAddress, void* pvStackAddress, u64 qwStackSize) {
    kMemSet(stContext.vqRegister, 0, sizeof(stContext.vqRegister));

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