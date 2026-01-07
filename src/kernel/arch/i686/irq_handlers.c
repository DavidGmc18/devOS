#include "irq_handlers.h"
#include "irq.h"

void timer(Registers* regs) {
}

void i686_IRQ_Handlers() {
    i686_IRQ_RegisterHandler(0 , timer);
}