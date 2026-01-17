#include "time.h"
#include <arch/i686/rtc.h>
#include <string.h>
#include <stdbool.h>

const uint16_t __month_days[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const uint16_t __year_to_month_days[12] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

#define IS_LEAP(year) ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)

// TODO isdst
static void __rtc_to_tm(RTC_time* rtc, struct tm* tm) {
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

static const char __week_day_name[7][3] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char __month_name[12][3] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char __digits[16] = "0123456789ABCDEF";

static void __print_unsigned(char* buf, int num, int size, int radix, const char* end) {
    for (int i = size - 1; i >= 0; i--) {
        if (buf + i < end) {
            buf[i] = __digits[num % radix];
        }
        num /= radix;
    }
}

static int __asctime(char* buf, size_t bufsz, const struct tm* tm) {
    if (0 < bufsz) strncpy(buf + 0, __week_day_name[tm->tm_wday], (3 < (bufsz - 0)) ? 3 : (bufsz - 0));
    if (4 < bufsz) strncpy(buf + 4, __month_name[tm->tm_mon], (3 < (bufsz - 4)) ? 3 : (bufsz - 4));
    __print_unsigned(buf + 8, tm->tm_mday, 2, 10, buf + bufsz);
    __print_unsigned(buf + 11, tm->tm_hour, 2, 10, buf + bufsz);
    __print_unsigned(buf + 14, tm->tm_min, 2, 10, buf + bufsz);
    __print_unsigned(buf + 17, tm->tm_sec, 2, 10, buf + bufsz);
    __print_unsigned(buf + 20, tm->tm_year + 1900, 4, 10, buf + bufsz);

    if (3 < bufsz) buf[3]  = ' ';
    if (7 < bufsz) buf[7]  = ' ';
    if (10 < bufsz) buf[10] = ' ';
    if (13 < bufsz) buf[13] = ':';
    if (16 < bufsz) buf[16] = ':';
    if (19 < bufsz) buf[19] = ' ';

    return bufsz < 24 ? bufsz : 24;
}

int asctime(char* buf, size_t bufsz, const struct tm* tm) {
    __asctime(buf, bufsz, tm);
    if (24 < bufsz) buf[24] = '\n';
    if (25 < bufsz) buf[25] = '\0';

    return bufsz < 26 ? bufsz : 26;
}

size_t strftime(char* str, size_t count, const char* format, const struct tm* tm) {
    bool specifier = false;
    int i = 0;

    while (*format && i < count) {
        if (!specifier) {
            switch (*format) {
                case '%':
                    specifier = true;
                    break;

                default:
                    str[i++] = *format;
                    break;
            }
        } else {
            switch (*format) {
                case '%':
                    str[i++] = '%';
                    break;

                case 'Y':
                    __print_unsigned(str+i, tm->tm_year+1900, 4, 10, str+count);
                    i += 4;
                    break;

                case 'y':
                    __print_unsigned(str+i, tm->tm_year+1900, 2, 10, str+count);
                    i += 2;
                    break;
                
                case 'b':
                case 'h':
                    for (int j = 0; j < 3 && i < count; j++) {
                        str[i++] = __month_name[tm->tm_mon][j];
                    }
                    break;

                case 'm':
                    __print_unsigned(str+i, tm->tm_mon+1, 2, 10, str+count);
                    i += 2;
                    break;

                case 'j':
                    __print_unsigned(str+i, tm->tm_yday+1, 3, 10, str+count);
                    i += 3;
                    break;

                case 'd':
                    __print_unsigned(str+i, tm->tm_mday, 2, 10, str+count);
                    i += 2;
                    break;

                case 'a':
                    for (int j = 0; j < 3 && i < count; j++) {
                        str[i++] = __week_day_name[tm->tm_wday][j];
                    }
                    break;

                case 'w':
                    __print_unsigned(str+i, tm->tm_wday, 1, 10, str+count);
                    i += 1;
                    break;

                case 'H':
                    __print_unsigned(str+i, tm->tm_hour, 2, 10, str+count);
                    i += 2;
                    break;
                    
                case 'I':
                    __print_unsigned(str+i, ((tm->tm_hour+11)%12)+1, 2, 10, str+count);
                    i += 2;
                    break;

                case 'M':
                    __print_unsigned(str+i, tm->tm_min, 2, 10, str+count);
                    i += 2;
                    break;

                case 'S':
                    __print_unsigned(str+i, tm->tm_sec, 2, 10, str+count);
                    i += 2;
                    break;

                case 'x':
                    i += __asctime(str+i, count-i, tm);
                    break;

                default:
                    str[i-1] = '%';
                    str[i++] = *format;
            }
            specifier = false;
        }
        format++;

        if (i < count) {
            str[i] = '\0';
        }
    }
}