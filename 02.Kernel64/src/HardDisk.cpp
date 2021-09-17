#include "HardDisk.hpp"
#include "Assembly.hpp"
#include "Utility.hpp"
#include "Console.hpp"

static HDDManager gs_stHDDManager;

bool kInitializeHDD() {
    gs_stHDDManager.bPrimaryInterruptOccur = false;
    gs_stHDDManager.bSecondaryInterruptOccur = false;

    kOutPortByte(HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0);
    kOutPortByte(HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0);

    if(!kReadHDDInformation(true, true, gs_stHDDManager.stHDDInformation)) {
        gs_stHDDManager.bHDDDectected = false;
        gs_stHDDManager.bCanWrite = false;
        return false;
    }

    gs_stHDDManager.bHDDDectected = true;
    gs_stHDDManager.bCanWrite = kMemCmp(gs_stHDDManager.stHDDInformation.vwModelNumber, "QEMU", 4) == 0;
    return true;
}

static u8 kReadHDDStatus(bool bPrimary) {
    return bPrimary ?
        kInPortByte(HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_STATUS) :
        kInPortByte(HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_STATUS);
}

static bool kWaitForHDDNoBusy(bool bPrimary) {
    u64 qwStartTickCount = kGetTickCount();
    while((kGetTickCount() - qwStartTickCount) <= HDD_WAITTIME) {
        u8 bStatus = kReadHDDStatus(bPrimary);
        if((bStatus & HDD_STATUS_BUSY) != HDD_STATUS_BUSY) return true;
        kSleep(1);
    }
    return false;
}

static bool kWaitForHDDReady(bool bPrimary) {
    u64 qwStartTickCount = kGetTickCount();
    while((kGetTickCount() - qwStartTickCount) <= HDD_WAITTIME) {
        u8 bStatus = kReadHDDStatus(bPrimary);
        if((bStatus & HDD_STATUS_READY) == HDD_STATUS_READY) return true;
        kSleep(1);
    }
    return false;
}

void kSetHDDInterruptFlag(bool bPrimary, bool bFlag) {
    if(bPrimary) gs_stHDDManager.bPrimaryInterruptOccur = bFlag;
    else gs_stHDDManager.bSecondaryInterruptOccur = bFlag;
}

static bool kWaitForHDDInterrupt(bool bPrimary) {
    u64 qwStartTickCount = kGetTickCount();
    while((kGetTickCount() - qwStartTickCount) <= HDD_WAITTIME) {
        if(bPrimary && gs_stHDDManager.bPrimaryInterruptOccur) return true;
        if(!bPrimary && gs_stHDDManager.bSecondaryInterruptOccur) return true;
        kSleep(10);
    }
    return false;
}

bool kReadHDDInformation(bool bPrimary, bool bMaster, HDDInformation &pstHDDInformation) {
    u16 wPortBase = bPrimary ? HDD_PORT_PRIMARYBASE : HDD_PORT_SECONDARYBASE;
    gs_stHDDManager.stMutex.lock();
    if(!kWaitForHDDNoBusy(bPrimary)) {
        gs_stHDDManager.stMutex.unlock();
        return false;
    }
    u8 bDriveFlag = bMaster ? HDD_DRIVEANDHEAD_LBA : HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    kOutPortByte(wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag);

    if(!kWaitForHDDReady(bPrimary)) {
        gs_stHDDManager.stMutex.unlock();
        return false;
    }

    kSetHDDInterruptFlag(bPrimary, false);

    kOutPortByte(wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_IDENTIFY);
    bool bWaitResult = kWaitForHDDInterrupt(bPrimary);
    u8 bStatus = kReadHDDStatus(bPrimary);
    if(!bWaitResult || (bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR) {
        gs_stHDDManager.stMutex.unlock();
        return false;
    }

    for(int i = 0; i < 512 / 2; i++)
        ((u16*)(&pstHDDInformation))[i] = kInPortWord(wPortBase + HDD_PORT_INDEX_DATA);

    kSwapByteInWord(pstHDDInformation.vwModelNumber, sizeof(pstHDDInformation.vwModelNumber) / 2);
    kSwapByteInWord(pstHDDInformation.vwSerialNumber, sizeof(pstHDDInformation.vwSerialNumber) / 2);

    gs_stHDDManager.stMutex.unlock();
    return true;
}

static void kSwapByteInWord(u16 *pwData, int iWordCount) {
    for(int i = 0; i < iWordCount; i++) {
        u16 wTemp = pwData[i];
        pwData[i] = (wTemp >> 8) | (wTemp << 8);
    }
}

int kReadHDDSector(bool bPrimary, bool bMaster, u32 dwLBA, int iSectorCount, char *pcBuffer) {
    if(!gs_stHDDManager.bHDDDectected ||
        (iSectorCount <= 0) || (256 < iSectorCount) ||
        ((dwLBA + iSectorCount) >= gs_stHDDManager.stHDDInformation.dwTotalSector)) return 0;
    u16 wPortBase = bPrimary ? HDD_PORT_PRIMARYBASE : HDD_PORT_SECONDARYBASE;
    gs_stHDDManager.stMutex.lock();
    if(!kWaitForHDDNoBusy(bPrimary)) {
        gs_stHDDManager.stMutex.unlock();
        return false;
    }

    kOutPortByte(wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount);
    kOutPortByte(wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA);
    kOutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8);
    kOutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERMSB, dwLBA >> 16);

    u8 bDriveFlag = bMaster ? HDD_DRIVEANDHEAD_LBA : HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    kOutPortByte(wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag | (dwLBA >> 24) & 0x0F);

    if(!kWaitForHDDReady(bPrimary)) {
        gs_stHDDManager.stMutex.unlock();
        return false;
    }

    kSetHDDInterruptFlag(bPrimary, false);

    kOutPortByte(wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_READ);

    int i;
    long lReadCount = 0;
    for(i = 0; i < iSectorCount; i++) {
        u8 bStatus = kReadHDDStatus(bPrimary);
        if((bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR) {
            kPrintf("Error Occur\n");
            gs_stHDDManager.stMutex.unlock();
            return i;
        }
        if((bStatus & HDD_STATUS_DATAREQUEST) != HDD_STATUS_DATAREQUEST) {
            bool bWaitResult = kWaitForHDDInterrupt(bPrimary);
            kSetHDDInterruptFlag(bPrimary, false);
            if(!bWaitResult) {
                kPrintf("Interrupt Not Occur\n");
                gs_stHDDManager.stMutex.unlock();
                return false;
            }
        }

        for(int j = 0; j < 512 / 2; j++)
            ((u16*) pcBuffer)[lReadCount++] = kInPortWord(wPortBase + HDD_PORT_INDEX_DATA);
    }

    gs_stHDDManager.stMutex.unlock();
    return i;
}

int kWriteHDDSector(bool bPrimary, bool bMaster, u32 dwLBA, int iSectorCount, char* pcBuffer) {
    if(!gs_stHDDManager.bCanWrite ||
        (iSectorCount <= 0) || (256 < iSectorCount) ||
        ((dwLBA + iSectorCount) >= gs_stHDDManager.stHDDInformation.dwTotalSector)) return 0;
    u16 wPortBase = bPrimary ? HDD_PORT_PRIMARYBASE : HDD_PORT_SECONDARYBASE;
    if(!kWaitForHDDNoBusy(bPrimary)) return false;
    gs_stHDDManager.stMutex.lock();

    kOutPortByte(wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount);
    kOutPortByte(wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA);
    kOutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8);
    kOutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERMSB, dwLBA >> 16);

    u8 bDriveFlag = bMaster ? HDD_DRIVEANDHEAD_LBA : HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    kOutPortByte(wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag | (dwLBA >> 24) & 0x0F);

    if(!kWaitForHDDReady(bPrimary)) {
        gs_stHDDManager.stMutex.unlock();
        return false;
    }

    kOutPortByte(wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_WRITE);

    while(true) {
        u8 bStatus = kReadHDDStatus(bPrimary);
        if((bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR) {
            gs_stHDDManager.stMutex.unlock();
            return 0;
        }
        if((bStatus & HDD_STATUS_DATAREQUEST) == HDD_STATUS_DATAREQUEST) break;
        kSleep(1);
    }

    int i;
    long lReadCount = 0;
    for(i = 0; i < iSectorCount; i++) {
        kSetHDDInterruptFlag(bPrimary, false);
        for(int j = 0; j < 512 / 2; j++)
            kOutPortWord(wPortBase + HDD_PORT_INDEX_DATA, ((u16*) pcBuffer)[lReadCount++]);

        u8 bStatus = kReadHDDStatus(bPrimary);
        if((bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR) {
            gs_stHDDManager.stMutex.unlock();
            return i;
        }
        if((bStatus & HDD_STATUS_DATAREQUEST) != HDD_STATUS_DATAREQUEST) {
            bool bWaitResult = kWaitForHDDInterrupt(bPrimary);
            kSetHDDInterruptFlag(bPrimary, false);
            if(!bWaitResult) {
                gs_stHDDManager.stMutex.unlock();
                return false;
            }
        }
    }

    gs_stHDDManager.stMutex.unlock();
    return i;
}