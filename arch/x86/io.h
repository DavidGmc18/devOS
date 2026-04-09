#pragma once

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %1, %0" :: "Nd"(port), "a"(value));
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %1, %0" :: "Nd"(port), "a"(value));
}

static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %1, %0" :: "Nd"(port), "a"(value));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait() {
    outb(0x80, 0);
}

static inline void halt() {
    __asm__ volatile ("hlt" ::: "memory");
}

static inline void cli() {
    __asm__ volatile ("cli" ::: "memory");
}

static inline void sti() {
    __asm__ volatile ("sti" ::: "memory");
}

static inline void set_rsp(void* rsp) {
    __asm__ volatile("mov %0, %%rsp" :: "r"(rsp) : "memory");
}