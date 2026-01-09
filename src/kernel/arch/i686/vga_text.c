#include "vga_text.h"
#include "io.h"

uint16_t VGA_ScreenWidth;
uint16_t VGA_ScreenHeight;
uint8_t* VGA_ScreenBuffer;

void VGA_scr(uint16_t ScreenWidth, uint16_t ScreenHeight, uint8_t* ScreenBuffer) {
    VGA_ScreenWidth = ScreenWidth;
    VGA_ScreenHeight = ScreenHeight;
    VGA_ScreenBuffer = ScreenBuffer;
}

uint16_t VGA_Get_ScreenWidth(void){
    return VGA_ScreenWidth;
}

uint16_t VGA_Get_ScreenHeight(void){
    return VGA_ScreenHeight;
}

uint8_t* VGA_Get_ScreenBuffer(void){
    return VGA_ScreenBuffer;
}

void VGA_clrscr(uint8_t clear_color) {
    for (uint16_t y = 0; y < VGA_ScreenHeight; y++) {
        for (uint16_t x = 0; x < VGA_ScreenWidth; x++) {
            VGA_putchr(x, y, '\0');
            VGA_putcolor(x, y, clear_color);
        }
    }

    // width = 0;
    // height = 0;
    // VGA_setcursor(g_ScreenX, g_ScreenY);
}

void VGA_putchr(uint16_t x, uint16_t y, char c) {
    VGA_ScreenBuffer[2 * (y * VGA_ScreenWidth + x)] = c;
}

void VGA_putcolor(uint16_t x, uint16_t y, uint8_t color) {
    VGA_ScreenBuffer[2 * (y * VGA_ScreenWidth + x) + 1] = color;
}

char VGA_getchr(uint16_t x, uint16_t y) {
    return VGA_ScreenBuffer[2 * (y * VGA_ScreenWidth + x)];
}

uint8_t VGA_getcolor(uint16_t x, uint16_t y) {
    return VGA_ScreenBuffer[2 * (y * VGA_ScreenWidth + x) + 1];
}