#pragma once

#include <stdint.h>

typedef enum {
    RTC_SEC =       0x00,
    RTC_MIN =       0x02,
    RTC_HOUR =      0x04,
    RTC_WDAY =      0x06,
    RTC_MDAY =      0x07,
    RTC_MONTH =     0x08,
    RTC_YEAR =      0x09,
    RTC_CENTURY =   0x32,
} RTC_Register;

uint8_t i686_RTC_get(RTC_Register reg);

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t wday;
    uint8_t mday;
    uint8_t month;
    uint8_t year;
    uint8_t century;
} RTC_time;

void i686_RTC_time(RTC_time* time);