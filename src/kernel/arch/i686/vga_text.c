#include "vga_text.h"
#include "io.h"

uint16_t ScreenWidth;
uint16_t ScreenHeight;

uint8_t* ScreenBuffer;

uint16_t ScreenX = 0;
uint16_t ScreenY = 0;

const uint8_t DEFAULT_COLOR = 0x7;

void VGA_Initialize(uint16_t Width, uint16_t Height, uint8_t* Buffer) {
    ScreenWidth = Width;
    ScreenHeight = Height;
    ScreenBuffer = Buffer;
}

void VGA_setcursor(int x, int y) {
    int pos = y * ScreenWidth + x;

    i686_outb(0x3D4, 0x0F);
    i686_outb(0x3D5, (uint8_t)(pos & 0xFF));
    i686_outb(0x3D4, 0x0E);
    i686_outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void VGA_clrscr() {
    for (uint16_t y = 0; y < ScreenHeight; y++) {
        for (uint16_t x = 0; x < ScreenWidth; x++) {
            VGA_putchr(x, y, '\0');
            VGA_putcolor(x, y, DEFAULT_COLOR);
        }
    }

    ScreenX = 0;
    ScreenY = 0;
    VGA_setcursor(ScreenX, ScreenY);
}

void VGA_scrollback(int lines) {
    for (int y = lines; y < ScreenHeight; y++) {
        for (int x = 0; x < ScreenWidth; x++) {
            VGA_putchr(x, y - lines, VGA_getchr(x, y));
            VGA_putcolor(x, y - lines, VGA_getcolor(x, y));
        }
    }

    for (int y = ScreenHeight - lines; y < ScreenHeight; y++) {
        for (int x = 0; x < ScreenWidth; x++) {
            VGA_putchr(x, y, '\0');
            VGA_putcolor(x, y, DEFAULT_COLOR);
        }
    }

    ScreenY -= lines;
}

void VGA_putchr(uint16_t x, uint16_t y, char ch) {
    ScreenBuffer[2 * (y * ScreenWidth + x)] = ch;
}

void VGA_putcolor(uint16_t x, uint16_t y, uint8_t color) {
    ScreenBuffer[2 * (y * ScreenWidth + x) + 1] = color;
}

char VGA_getchr(uint16_t x, uint16_t y) {
    return ScreenBuffer[2 * (y * ScreenWidth + x)];
}

uint8_t VGA_getcolor(uint16_t x, uint16_t y) {
    return ScreenBuffer[2 * (y * ScreenWidth + x) + 1];
}

void VGA_putc(char ch) {
    switch (ch) {
        case '\n':
            ScreenX = 0;
            ScreenY++;
            break;
    
        case '\t':
            for (int i = 0; i < 4 - (ScreenX % 4); i++) {
                VGA_putc(' ');
            }
            break;

        case '\r':
            ScreenX = 0;
            break;

        default:
            VGA_putchr(ScreenX, ScreenY, ch);
            ScreenX++;
            break;
    }

    if (ScreenX >= ScreenWidth){
        ScreenY++;
        ScreenX = 0;
    }

    if (ScreenY >= ScreenHeight) {
        VGA_scrollback(1);
    }

    VGA_setcursor(ScreenX, ScreenY);
}

void VGA_puts(const char* str) {
    while (*str) {
        VGA_putc(*str);
        str++;
    }
}

void VGA_putn(const char* str, size_t size) {
    for (size_t i = 0; i < size; i++) {
        VGA_putc(str[i]);
    }
}