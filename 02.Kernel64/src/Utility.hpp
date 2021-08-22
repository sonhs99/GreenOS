#pragma once

#include <stdarg.h>
#include "Types.hpp"

void kMemSet(void* pvDestination, u8 bData, int iSize);
int	 kMemCpy(void* pvDestination, const void* pvSource, int iSize);
int	 kMemCmp(const void* pvDestination, const void* pvSource, int iSize);
bool kSetInterruptFlag(bool bEnableInterrupt);
int	 kStrLen(const char * pcBuffer);

void kCheckTotalRAMSize();
u64	 kGetTotalRAMSize();
void kCheckTotalRAMSize();
void kReverseString(char *pcBuffer);
long kAToI(const char *pcBuffer, int iRadix);
u64	 kHexStringToQword(const char *pcBuffer);
long kDecimalStringToLong(const char *pcBuffer);
int	 kIToA(long lValue, char *pcBuffer, int iRadix);
int	 kHexToString(u64 qwValue, char *pcBuffer);
int	 kDecimalToString(long lValue, char* pcBuffer);
int	 kSPrintf(char *pcBuffer, const char *pcFormatString, ...);
int	 kVSPrintf(char *pcBuffer, const char* pcFormatString, va_list ap);