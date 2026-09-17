#include "interrupts.h"
#include "process.h"

#define IDT_ENTRIES 256
#define KERNEL_CODE_SELECTOR 0x08

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define PIC_EOI      0x20

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

extern void irq0_stub(void);

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ __volatile__(
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

static void idt_set_gate(uint8_t vector, uint32_t handler)
{
    idt[vector].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[vector].selector    = KERNEL_CODE_SELECTOR;
    idt[vector].zero        = 0;
    idt[vector].type_attr   = 0x8E;
    idt[vector].offset_high = (uint16_t)((handler >> 16) & 0xFFFF);
}

static void pic_remap(void)
{
    /* Start PIC initialisation. */
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    /* Master IRQs -> 0x20-0x27, slave -> 0x28-0x2F. */
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    /* Tell the PICs how they are connected. */
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    /* 8086 mode. */
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    /*
     * Enable only IRQ0 (PIT) on the master PIC.
     * Keep the slave PIC masked for now.
     */
    outb(PIC1_DATA, 0xFE);
    outb(PIC2_DATA, 0xFF);
}

void interrupts_init(void)
{
    uint32_t i;

    for (i = 0; i < IDT_ENTRIES; i++) {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].type_attr = 0;
        idt[i].offset_high = 0;
    }

    /* IRQ0 appears at interrupt vector 0x20 after PIC remapping. */
    idt_set_gate(0x20, (uint32_t)irq0_stub);

    idt_ptr.limit = (uint16_t)(sizeof(idt) - 1);
    idt_ptr.base = (uint32_t)&idt;

    __asm__ __volatile__("lidt %0" : : "m"(idt_ptr));

    pic_remap();
}

void interrupts_enable(void)
{
    __asm__ __volatile__("sti");
}

void interrupts_disable(void)
{
    __asm__ __volatile__("cli");
}

