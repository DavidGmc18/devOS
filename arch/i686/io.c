#include "io.h"

void i686_outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %1, %0" :: "Nd"(port), "a"(value));
}

void i686_outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %1, %0" :: "Nd"(port), "a"(value));
}

void i686_outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %1, %0" :: "Nd"(port), "a"(value));
}

uint8_t i686_inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint16_t i686_inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint32_t i686_inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#define UNUSED_PORT     0x80

void i686_iowait() {
    i686_outb(UNUSED_PORT, 0);
}