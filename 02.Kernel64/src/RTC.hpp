#pragma once

#include "Types.hpp"

#define RTC_CMOSADDRESS         0x70
#define RTC_CMOSDATA            0x71

#define RTC_ADDRESS_SECOND      0x00
#define RTC_ADDRESS_MINUTE      0x02
#define RTC_ADDRESS_HOUR        0x04
#define RTC_ADDRESS_DAYOFWEEK   0x06
#define RTC_ADDRESS_DAYOFMONTH  0x07
#define RTC_ADDRESS_MONTH       0x08
#define RTC_ADDRESS_YEAR        0x09

#define RTC_BCDTOBYNARY( X )    ( ( ( ( X ) >> 4 ) * 10 ) + ( ( X ) & 0x0F ) )

void kReadRTCTime(u8 &pbHour, u8 &pbMinute, u8 &pbSecond);
void kReadRTCDate(u16 &pwYear, u8 &pbMonth, u8 &pbDayOfMonth, u8 &pbDayOfWeek);
char* kConvertDayOfWeekToString(u8 bDayOfWeek);