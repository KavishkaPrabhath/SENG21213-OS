#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "../include/types.h"

/* Initialise the IDT and remap/configure the 8259 PIC. */
void interrupts_init(void);

/* Enable and disable CPU hardware interrupts. */
void interrupts_enable(void);
void interrupts_disable(void);


#endif
