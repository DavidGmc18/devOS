#include "rtc.h"
#include "i686.h"

#define RTC_COMMAND_PORT    0x70
#define RTC_DATA_PORT       0x71

uint8_t i686_RTC_get(RTC_Register reg) {
    i686_outb(RTC_COMMAND_PORT, reg);
    i686_iowait();
    uint8_t bcd = i686_inb(RTC_DATA_PORT);
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void i686_RTC_time(RTC_time* time) {
    time->sec = i686_RTC_get(RTC_SEC);
    time->min = i686_RTC_get(RTC_MIN);
    time->hour = i686_RTC_get(RTC_HOUR);
    time->wday = i686_RTC_get(RTC_WDAY);
    time->mday = i686_RTC_get(RTC_MDAY);
    time->month = i686_RTC_get(RTC_MONTH);
    time->year = i686_RTC_get(RTC_YEAR);
    time->century = i686_RTC_get(RTC_CENTURY);
}