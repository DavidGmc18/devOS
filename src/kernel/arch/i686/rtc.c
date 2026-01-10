#include "rtc.h"
#include "io.h"

#define RTC_COMMAND_PORT    0x70
#define RTC_DATA_PORT       0x71

uint8_t bcd_to_decimal(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t rtc(RTC_Register reg) {
    i686_outb(RTC_COMMAND_PORT, reg);
    i686_iowait();
    uint8_t bcd = i686_inb(RTC_DATA_PORT);
    return bcd_to_decimal(bcd);
}