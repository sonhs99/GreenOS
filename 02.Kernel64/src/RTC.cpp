#include "RTC.hpp"
#include "Assembly.hpp"

void kReadRTCTime(u8 &pbHour, u8 &pbMinute, u8 &pbSecond) {
    u8 bData;

    kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_HOUR);
    bData = kInPortByte(RTC_CMOSDATA);
    pbHour = RTC_BCDTOBYNARY(bData);
    
    kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MINUTE);
    bData = kInPortByte(RTC_CMOSDATA);
    pbMinute = RTC_BCDTOBYNARY(bData);
    
    kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_SECOND);
    bData = kInPortByte(RTC_CMOSDATA);
    pbSecond = RTC_BCDTOBYNARY(bData);
}

void kReadRTCDate(u16 &pwYear, u8 &pbMonth, u8 &pbDayOfMonth, u8 &pbDayOfWeek) {
    u8 bData;

    kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_YEAR);
    bData = kInPortByte(RTC_CMOSDATA);
    pwYear = RTC_BCDTOBYNARY(bData) + 2000;
    
    kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MONTH);
    bData = kInPortByte(RTC_CMOSDATA);
    pbMonth = RTC_BCDTOBYNARY(bData);
    
    kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFMONTH);
    bData = kInPortByte(RTC_CMOSDATA);
    pbDayOfMonth = RTC_BCDTOBYNARY(bData);
    
    kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFWEEK);
    bData = kInPortByte(RTC_CMOSDATA);
    pbDayOfWeek = RTC_BCDTOBYNARY(bData);
}

char* kConvertDayOfWeekToString(u8 bDayOfWeek) {
    static char *vpcDayOfWeekString[] = { "Error", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    if(bDayOfWeek >= 8) return vpcDayOfWeekString[0];
    return vpcDayOfWeekString[bDayOfWeek];
}