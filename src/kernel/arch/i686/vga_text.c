#include "vga_text.h"
#include "io.h"
#include "logger.h"
#include <stdbool.h>

uint16_t ScreenWidth;
uint16_t ScreenHeight;

uint8_t* ScreenBuffer;

uint16_t ScreenX = 0;
uint16_t ScreenY = 0;

const uint8_t DEFAULT_COLOR = 0x07;

uint8_t CursorColor = DEFAULT_COLOR;

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
            VGA_putcolor(ScreenX, ScreenY, CursorColor);
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

const uint8_t ansi_to_vga[] = {
    [0] =   0x0,  // Black
    [1] =   0x4,   // Red
    [2] =   0x2,   // Green
    [3] =   0x6,   // Yellow / Brown
    [4] =   0x1,   // Blue
    [5] =   0x5,   // Magenta
    [6] =   0x3,   // Cyan
    [7] =   0x7,   // White

    [8] =   0x8,   // Bright Black
    [9] =   0xC,   // Bright Red
    [10] =  0xA,   // Bright Green
    [11] =  0xE,   // Bright Yellow
    [12] =  0x9,   // Bright Blue
    [13] =  0xD,   // Bright Magenta
    [14] =  0xB,   // Bright Cyan
    [15] =  0xF,   // Bright White
};

void VGA_ansi_set(uint8_t ansi_code) {
    // FG
    if (ansi_code >= 30 && ansi_code < 38) {
        CursorColor &= 0xF0; 
        CursorColor |= ansi_to_vga[ansi_code - 30];
        return;
    }
    
    // FG
    if (ansi_code >= 90 && ansi_code < 98) {
        CursorColor &= 0xF0; 
        CursorColor |= ansi_to_vga[ansi_code - 90 + 8];
        return;
    }
    
    // BG
    if (ansi_code >= 40 && ansi_code < 48) {
        CursorColor &= 0x0F; 
        CursorColor |= (ansi_to_vga[ansi_code - 40] << 4);
        return;
    }

    // BG
    if (ansi_code >= 100 && ansi_code < 108) {
        CursorColor &= 0x0F; 
        CursorColor |= (ansi_to_vga[ansi_code - 100 + 8] << 4);
        return;
    }

    // Reset
    if (ansi_code == 0) {
        CursorColor = DEFAULT_COLOR;
        return;
    }
}

// TODO printf remainder after ansi end and infact even before or separate ascii func?
void VGA_putn(const char* str, size_t size) {
    if (str[0] == '\033' && str[1] == '[') {
        int end = 0;
        for (int i = 2; i < size; i++) {
            if (str[i] == 'm') {
                end = i;
                break;
            }
        }
        if (end == 0) goto normal;

        uint8_t ansi_code = 0;
        for (int i = 2; i <= end; i++) {
            if (str[i] == ';' || str[i] == 'm') {
                VGA_ansi_set(ansi_code);
                ansi_code = 0;
            } else if (str[i] >= '0' && str[i] <= '9') {
                ansi_code *= 10;
                ansi_code += str[i] - '0';
            }
        }

        return;
    }

    normal:
    for (size_t i = 0; i < size; i++) {
        VGA_putc(str[i]);
    }
}