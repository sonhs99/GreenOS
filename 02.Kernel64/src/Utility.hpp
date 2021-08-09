#pragma once

#include "Types.hpp"

void kMemSet(void* pvDestination, u8 bData, int iSize);
int kMemCpy(void* pvDestination, const void* pvSource, int iSize);
int kMemCmp(const void* pvDestination, const void* pvSource, int iSize);