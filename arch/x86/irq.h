#pragma once

#include "interrupt.h"

void irq_init();

void irq_dispatch(struct regs *r);