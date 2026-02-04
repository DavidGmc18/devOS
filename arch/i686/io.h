#pragma once

#include <stdint.h>

typedef uint16_t port_t;

#define NULL_PORT ((port_t)(0))

void i686_outb(port_t port, uint8_t value);
void i686_outw(port_t port, uint16_t value);
void i686_outl(port_t port, uint32_t value);

uint8_t i686_inb(port_t port);
uint16_t i686_inw(port_t port);
uint32_t i686_inl(port_t port);

uint8_t __attribute__((cdecl)) i686_EnableInterrupts();
uint8_t __attribute__((cdecl)) i686_DisableInterrupts();

void i686_iowait();
void __attribute__((cdecl)) i686_Panic();