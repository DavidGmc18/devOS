#pragma once

#include <stdint.h>

void VGA_scr(uint16_t ScreenWidth, uint16_t ScreenHeight, uint8_t* ScreenBuffer);

uint16_t VGA_Get_ScreenWidth(void);
uint16_t VGA_Get_ScreenHeight(void);
uint8_t* VGA_Get_ScreenBuffer(void);

void VGA_clrscr(uint8_t clear_color);

void VGA_putchr(uint16_t x, uint16_t y, char c);
void VGA_putcolor(uint16_t x, uint16_t y, uint8_t color);
char VGA_getchr(uint16_t x, uint16_t y);
uint8_t VGA_getcolor(uint16_t x, uint16_t y);