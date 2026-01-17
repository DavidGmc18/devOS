#pragma once

#include <stdint.h>
#include <stddef.h>

typedef size_t time_t;

struct tm {
    int tm_sec;     // seconds after the minute – [0, 59]
    int tm_min;     // minutes after the hour – [0, 59]
    int tm_hour;    // hours since midnight – [0, 23]
    int tm_mday;    // day of the month – [1, 31]
    int tm_mon;     // months since January – [0, 11]
    int tm_year;    // years since 1900
    int tm_wday;    // days since Sunday – [0, 6]
    int tm_yday;    // days since January 1 – [0, 365]
    int tm_isdst;   // daylight saving time flag
};

void time_tm(struct tm* tm);

time_t mktime(struct tm* tm);
time_t time(time_t* tloc);

#define ASCTIME_BUFSZ 26
int asctime(char* buf, size_t bufsz, const struct tm* tm);

size_t strftime(char* str, size_t count, const char* format, const struct tm* tm);
// %%, \n, \t
// Y, y
// b, m
// d, e, a, w
// H, M, S