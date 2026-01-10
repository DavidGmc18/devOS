#pragma once

#include <stdint.h>

typedef enum {
    RTC_SECONDS =     0x00,
    RTC_MINUTES =     0x02,
    RTC_HOURS =       0x04,
    RTC_WEEKDAY =     0x06,
    RTC_DAY =         0x07,
    RTC_MONTH =       0x08,
    RTC_YEAR =        0x09,
    RTC_CENTURY =     0x32,
} RTC_Register;

uint8_t rtc(RTC_Register reg);