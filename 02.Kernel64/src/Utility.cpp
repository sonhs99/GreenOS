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

int kStrLen(const char *pcBuffer) {
	int i;
	for(i = 0; pcBuffer[i] != '\0'; i++);
	return i;
}

static u64 gs_qwTotalRAMMBSize = 0;

void kCheckTotalRAMSize() {
	u32 *pdwCurrentAddress = (u32 *)0x4000000;
	while (true){
		u32 dwPreviousValue = *pdwCurrentAddress;
		*pdwCurrentAddress = 0x12345678;
		if (*pdwCurrentAddress != 0x12345678) break;
		pdwCurrentAddress += (0x100000 / 4);
	}
	gs_qwTotalRAMMBSize = u64(pdwCurrentAddress) / 0x100000;
}

u64 kGetTotalRAMSize() {
	return gs_qwTotalRAMMBSize;
}

long kAToI(const char* pcBuffer, int iRadix) {
	switch(iRadix) {
	case 16:
		return kHexStringToQword(pcBuffer);
	case 10:
	default:
		return kDecimalStringToLong(pcBuffer);
	}
}

u64 kHexStringToQword(const char *pcBuffer) {
	u64 qwValue = 0;
	for(int i = 0; pcBuffer[i] != '\0'; i++) {
		qwValue *= 16;
		if(('A' <= pcBuffer[i]) && (pcBuffer[i] <= 'Z'))
			qwValue += (pcBuffer[i] - 'A') + 10;
		else if (('a' <= pcBuffer[i]) && (pcBuffer[i] <= 'z'))
			qwValue += (pcBuffer[i] - 'a') + 10;
		else
			qwValue += pcBuffer[i] - '0';
	}
	return qwValue;
}

long kDecimalStringToLong(const char *pcBuffer) {
	long lValue = 0;
	int i = 0;

	if(pcBuffer[0] == '-') i = 1;
	for(; pcBuffer[i] != '\0'; i++) {
		lValue *= 10;
		lValue += pcBuffer[i] - '0';
	}

	if (pcBuffer[0] == '-') lValue = -lValue;
	return lValue;
}

int kIToA(long lValue, char *pcBuffer, int iRadix) {
	switch(iRadix) {
	case 16:
		return kHexToString(lValue, pcBuffer);
	case 10:
	default:
		return kDecimalToString(lValue, pcBuffer);
	}
}

int kHexToString(u64 qwValue, char* pcBuffer) {
	if(qwValue == 0) {
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	int i;
	for(i = 0; qwValue > 0; i++) {
		u64 qwCurrentValue = qwValue % 16;
		if(qwCurrentValue >= 10)
			pcBuffer[i] = 'A' + (qwCurrentValue - 10);
		else pcBuffer[i] = '0' + qwCurrentValue;
		qwValue /= 16;
	}

	pcBuffer[i] = '\0';
	kReverseString(pcBuffer);
	return i;
}

int kDecimalToString(long lValue, char *pcBuffer) {
	int i = 0;

	if (lValue == 0) {
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}
	if (lValue < 0) {
		i = 1;
		pcBuffer[0] = '-';
		lValue = -lValue;
	}

	for (; lValue > 0; i++, lValue /= 10)
		pcBuffer[i] = '0' + lValue % 10;

	pcBuffer[i] = '\0';
	if(pcBuffer[0] == 0) kReverseString(pcBuffer + 1);
	else kReverseString(pcBuffer);
	return i;
}

void kReverseString(char * pcBuffer) {
	int iLength = kStrLen(pcBuffer);
	for(int i = 0; i < iLength / 2; i++) {
		char cTemp = pcBuffer[i];
		pcBuffer[i] = pcBuffer[iLength - 1 - i];
		pcBuffer[iLength - 1 - i] = cTemp;
	}
}

int kSPrintf(char* pcBuffer, const char* pcFormatString, ...) {
	va_list ap;

	va_start(ap, pcFormatString);
	int iReturn = kVSPrintf(pcBuffer, pcFormatString, ap);
	va_end(ap);

	return iReturn;
}

int kVSPrintf(char *pcBuffer, const char* pcFormatString, va_list ap) {
	char* pcCopyString;
	int iCopyLength;
	int iValue;
	u32 dwValue;
	u64 qwValue;

	int iFormatLength = kStrLen(pcFormatString);
	int iBufferIndex = 0;
	for(u64 i = 0; i < iFormatLength; i++) {
		if(pcFormatString[i] == '%') {
			i++;
			switch(pcFormatString[i]) {
			case 's':
				pcCopyString = (char*)(va_arg(ap, char*));
				iCopyLength = kStrLen(pcCopyString);
				kMemCpy(pcBuffer + iBufferIndex, pcCopyString, iCopyLength);
				iBufferIndex += iCopyLength;
				break;

			case 'c':
				pcBuffer[iBufferIndex] = char(va_arg(ap, int));
				iBufferIndex++;
				break;
			
			case 'd':
			case 'i':
				iValue = int(va_arg(ap, int));
				iBufferIndex += kIToA(iValue, pcBuffer + iBufferIndex, 10);
				break;

			case 'X':
			case 'x':
				dwValue = u32(va_arg(ap, u32)) & 0xFFFFFFFF;
				iBufferIndex += kIToA(dwValue, pcBuffer + iBufferIndex, 16);
				break;

			case 'q':
			case 'Q':
			case 'p':
				qwValue = u64(va_arg(ap, u64));
				iBufferIndex += kIToA(qwValue, pcBuffer + iBufferIndex, 16);
				break;

			default:
				pcBuffer[iBufferIndex++] = pcFormatString[i];
			}
		} else pcBuffer[iBufferIndex++] = pcFormatString[i];
	}

	pcBuffer[iBufferIndex] = '\0';
	return iBufferIndex;
}