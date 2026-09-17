#include "timer.h"

/* PIT I/O ports */
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_BASE_FREQUENCY 1193182

/* Write one byte to an I/O port. */
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ __volatile__(
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

/*
 * Configure PIT channel 0.
 * Mode 3 = square-wave generator.
 */
void timer_init(uint32_t frequency)
{
    uint32_t divisor;

    if (frequency == 0)
        return;

    divisor = PIT_BASE_FREQUENCY / frequency;

    /* Channel 0, lobyte/hibyte, mode 3, binary. */
    outb(PIT_COMMAND, 0x36);

    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
}
