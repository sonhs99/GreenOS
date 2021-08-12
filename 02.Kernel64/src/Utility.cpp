#include "Utility.hpp"
#include "Assembly.hpp"

void kMemSet(void* pvDestination, u8 bData, int iSize) {
    for(int i = 0; i < iSize; i++)
        ((char*) pvDestination)[i] = bData;
}

int kMemCpy(void* pvDestination, const void* pvSource, int iSize) {
    for(int i = 0; i < iSize; i++)
        ((char*) pvDestination)[i] = ((char*) pvSource)[i];
    return iSize;
}

int kMemCmp(const void* pvDestination, const void* pvSource, int iSize) {
    char cTemp;
    for(int i = 0; i < iSize; i++)
        if(cTemp = (((char*) pvDestination)[i] - ((char*) pvSource)[i]))
            return cTemp;
    return 0;
}

bool kSetInterruptFlag(bool bEnableInterrupt) {
    u64 qwRFLAGS = kReadFLAGS();
    if(bEnableInterrupt) kEnableInterrupt();
    else kDisableInterrupt();
    if(qwRFLAGS & 0x0200) return true;
    return false;
}