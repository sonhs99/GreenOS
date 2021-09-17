#pragma once

#include "Types.hpp"
#include "Sync.hpp"

#define HDD_PORT_PRIMARYBASE                0x1F0
#define HDD_PORT_SECONDARYBASE              0x170

#define HDD_PORT_INDEX_DATA                 0x00
#define HDD_PORT_INDEX_SECTORCOUNT          0x02
#define HDD_PORT_INDEX_SECTORNUMBER         0x03
#define HDD_PORT_INDEX_CYLINDERLSB          0x04
#define HDD_PORT_INDEX_CYLINDERMSB          0x05
#define HDD_PORT_INDEX_DRIVEANDHEAD         0x06
#define HDD_PORT_INDEX_STATUS               0x07
#define HDD_PORT_INDEX_COMMAND              0x07
#define HDD_PORT_INDEX_DIGITALOUTPUT        0x206

#define HDD_COMMAND_READ                    0x20
#define HDD_COMMAND_WRITE                   0x30
#define HDD_COMMAND_IDENTIFY                0xEC

#define HDD_STATUS_ERROR                    0x01
#define HDD_STATUS_INDEX                    0x02
#define HDD_STATUS_CORRECTEDDATA            0x04
#define HDD_STATUS_DATAREQUEST              0x08
#define HDD_STATUS_SEEKCOMLETE              0x10
#define HDD_STATUS_WRITEFAULT               0x20
#define HDD_STATUS_READY                    0x40
#define HDD_STATUS_BUSY                     0x80

#define HDD_DRIVEANDHEAD_LBA                0xE0
#define HDD_DRIVEANDHEAD_SLAVE              0x10

#define HDD_DIGITALOUTPUT_RESET             0x04
#define HDD_DIGITALOUTPUT_DISABLEINTERRUPT  0x01

#define HDD_WAITTIME                        500
#define HDD_MAXBUCKSECTORCOUNT              256

#pragma pack(push, 1)

struct HDDInformation {
    u16 wConfiguation;

    u16 wNumberOfCylinder;
    u16 wReserved1;

    u16 wNumberOfHead;
    u16 wUnformattedBytesPerTrack;
    u16 wUnformattedBytesPerSector;

    u16 wNumberOfSectorPerCylinder;
    u16 wInterSectorGap;
    u16 wBytesInPhaseLock;
    u16 wNumberOfVendorUniqueStatusWord;

    u16 vwSerialNumber[10];
    u16 wControllerType;
    u16 wBufferSize;
    u16 wNumberOfECCByte;
    u16 vwFirmwareRevision[4];

    u16 vwModelNumber[20];
    u16 vwReserved2[13];
    
    u32 dwTotalSector;
    u16 vwReserved3[196];
};

#pragma pack(pop)

struct HDDManager {
    bool bHDDDectected;
    bool bCanWrite;

    volatile bool bPrimaryInterruptOccur;
    volatile bool bSecondaryInterruptOccur;
    Mutex stMutex;
    HDDInformation stHDDInformation;
};

bool kInitializeHDD();
bool kReadHDDInformation(bool bPrimary, bool bMaster, HDDInformation &pstHDDInformation);
int kReadHDDSector(bool bPrimary, bool bMaster, u32 dwLBA, int iSectorCount, char* pcBuffer);
int kWriteHDDSector(bool bPrimary, bool bMaster, u32 dwLBA, int iSectorCount, char* pcBuffer);
void kSetHDDInterruptFlag(bool bPrimary, bool bFlag);

static void kSwapByteInWord(u16 *pwData, int iWordCount);
static u8 kReadHDDStatus(bool bPrimary);
static bool kIsHDDBusy(bool bPrimary);
static bool kIsHDDReady(bool bPrimary);
static bool kWaitForHDDNoBusy(bool bPrimary);
static bool kWaitForHDDReady(bool bPrimary);
static bool kWaitForHDDInterrupt(bool bPrimary);