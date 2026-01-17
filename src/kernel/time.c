#include "time.h"
#include <arch/i686/rtc.h>
#include <string.h>

const uint16_t __month_days[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const uint16_t __year_to_month_days[12] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

#define IS_LEAP(year) ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)

// TODO isdst
void __rtc_to_tm(RTC_time* rtc, struct tm* tm) {
    tm->tm_sec = rtc->sec;
    tm->tm_min = rtc->min;
    tm->tm_hour = rtc->hour;
    tm->tm_mday = rtc->mday;
    tm->tm_mon = rtc->month - 1;
    tm->tm_year = rtc->century * 100 + rtc->year;

    tm->tm_wday = (rtc->wday + 6) % 7; // instead of subtracting 1, 6 is added to avoid negative modulo operation

    // handle overflows
    if (tm->tm_sec > 59) {
        tm->tm_min += tm->tm_sec / 60;
        tm->tm_sec %= 60;
    }

    if (tm->tm_min > 59) {
        tm->tm_hour += tm->tm_min / 60;
        tm->tm_min %= 60;
    }

    if (tm->tm_hour > 23) {
        tm->tm_mday += tm->tm_hour / 24;
        tm->tm_hour %= 24;
    }

    if (tm->tm_mon > 11) {
        tm->tm_year += tm->tm_mon / 12;
        tm->tm_mon %= 12;
    }

    if (tm->tm_mday > __month_days[tm->tm_mon]) {
        if (tm->tm_mon == 1 && IS_LEAP(tm->tm_year)) { // is FEB and is leap year
            tm->tm_mday = 29;
        } else {
            tm->tm_mday = __month_days[tm->tm_mon];
        }
    }

    tm->tm_yday = __year_to_month_days[tm->tm_mon] + tm->tm_mday - 1;
    tm->tm_yday += (tm->tm_mon > 1 && IS_LEAP(tm->tm_year)); // add leap day

    // // This if statement is uncessary
    // if (tm->tm_yday > 364) {
    //     if (IS_LEAP(tm->tm_year)) {
    //         tm->tm_yday = 365;
    //     } else {
    //         tm->tm_yday = 364;
    //     }
    // }

    tm->tm_year -= 1900;
}

void time_tm(struct tm* tm) {
    RTC_time rtc;
    i686_RTC_time(&rtc);
    __rtc_to_tm(&rtc, tm);
}

time_t mktime(struct tm* tm) {
    uint16_t years = tm->tm_year - 70;
    time_t days = years * 365;
    days += years ? (years / 4) : (0);
    days -= (years + 70 - 1) / 100;
    days += (years + 370 - 1) / 400;

    days += tm->tm_yday;

    time_t seconds = days * 86400;
    seconds += tm->tm_hour * 3600;
    seconds += tm->tm_min * 60;
    seconds += tm->tm_sec;

    return seconds;
}

time_t time(time_t* tloc) {
    RTC_time rtc;
    struct tm tm;
    
    i686_RTC_time(&rtc);
    __rtc_to_tm(&rtc, &tm);

    time_t time = mktime(&tm);

    if (tloc != NULL) {
        *tloc = time;
    }

    return time;
}

const char __week_day_name[7][3] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

const char __month_name[12][3] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void __print_num(char* buffer, int num, int digits) {
    for (int i = digits - 1; i >= 0; i--) {
        buffer[i] = '0' + (num % 10);
        num /= 10;
    }
}

int asctime(char* buf, size_t bufsz, const struct tm* tm) {
    if (!buf || bufsz < ASCTIME_BUFSZ) {
        return -1;
    }

    strncpy(buf + 0, __week_day_name[tm->tm_wday], 3);
    strncpy(buf + 4, __month_name[tm->tm_mon], 3);
    __print_num(buf + 8, tm->tm_mday, 2);
    __print_num(buf + 11, tm->tm_hour, 2);
    __print_num(buf + 14, tm->tm_min, 2);
    __print_num(buf + 17, tm->tm_sec, 2);
    __print_num(buf + 20, tm->tm_year + 1900, 4);

    buf[3]  = ' ';
    buf[7]  = ' ';
    buf[10] = ' ';
    buf[13] = ':';
    buf[16] = ':';
    buf[19] = ' ';
    buf[24] = '\n';
    buf[25] = '\0';

    return 0;
}