#include "VGA.h"
#include <arch/x86/io.h>

#define WIDTH 80
#define HEIGHT 25

#define TAB_WIDTH 4

volatile unsigned short* buffer = (unsigned short*)0xB8000;
static int cx, cy;

#define DEFAULT_COLOR 0x07
#define DEFAULT_CHAR (DEFAULT_COLOR << 8)
unsigned char color = DEFAULT_COLOR;

static void set_cursor(int x, int y) {
    short pos = y * WIDTH + x;
    outb(0x3D4, 0x0F);
    io_wait();
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    io_wait();
    outb(0x3D4, 0x0E);
    io_wait();
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
    io_wait();
    cx = x;
    cy = y;
}

void VGA_init() {
    for (unsigned i = 0; i < WIDTH*HEIGHT; i++) {
        buffer[i] = DEFAULT_CHAR;
    }
    set_cursor(0, 0);
}

static void putc(char ch, int x, int y) {
    int position = y*WIDTH+x;
    if (position < 0 || position > (WIDTH*HEIGHT)) return;
    buffer[position] = (color << 8) | ch;
}

static void scroll(int y) {
    if (!y) return;

    int total_chars = WIDTH * HEIGHT;

    if (y > 0) {
        int shift = y * WIDTH;
        int end = total_chars - shift;
        if (end < 0) end = 0;

        for (int i = 0; i < end; i++) {
            buffer[i] = buffer[i+shift];
        }

        for (int i = end; i < total_chars; i++) {
            buffer[i] = DEFAULT_CHAR; 
        }
    } else {
        int shift = -y * WIDTH;
        if (shift > total_chars) shift = total_chars;

        for (int i = total_chars-1; i >= shift; i--) {
            buffer[i] = buffer[i-shift];
        }

        for (int i = 0; i < shift; i++) {
            buffer[i] = DEFAULT_CHAR; 
        }
    }

    set_cursor(cx, cy-y);
}

void VGA_putc(char ch) {
    switch (ch) {
        case '\n':
            cx = 0;
            cy++;
            break;

        case '\r':
            cx = 0;
            break;

        case '\t':
            cx += (TAB_WIDTH - (cx % TAB_WIDTH));
            break;

        default:
            putc(ch, cx++, cy);
    }

    if (cx >= WIDTH) {
        cx = 0;
        cy++;
    }

    if (cy >= HEIGHT) {
        scroll(1);
    }

    set_cursor(cx, cy);
}

void VGA_write(const char* str, unsigned long n) {
    for (unsigned long i = 0; i < n; i++) {
        VGA_putc(str[i]);
    }
}

static unsigned char get_level_color(int level) {
    static unsigned char colors[] = {0x40, 0x04, 0x0C, 0x0C, 0x0E, 0x0F};
    if (level < 0 || level >= sizeof(colors) / sizeof(colors[0])) 
        return DEFAULT_COLOR;
    return colors[level];
}

void VGA_log_write(int level, const char *str, unsigned long n) {
    color = get_level_color(level);
    VGA_write(str, n);
    color = DEFAULT_COLOR;
}