#include <stdint.h>

static uint8_t* screen_buffer = (uint8_t*)0xB8000;

static void put_chr(char ch, int x, int y) {
    screen_buffer[(y*80+x)*2] = ch;
}

static void put_str(const char* str, int x, int y) {
    while(*str) {
        put_chr(*str, x++, y);
        if (x > 80) {
            x = 0;
            y++;
        }
        str++;
    }
};

void __attribute__((noreturn, section(".entry"))) entry() {
    put_str("Long-Mode!!!       ", 0, 0);

    while (1) __asm__ volatile ("hlt" ::: "memory");
}