#ifndef PROCESS_H
#define PROCESS_H

#include "../include/types.h"

#define MAX_PROCESSES 16
#define STACK_SIZE    4096

/* Process states */
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} proc_state_t;

/* Process Control Block */
typedef struct pcb {
    uint32_t pid;
    proc_state_t state;

    uint32_t esp;                 /* Saved stack pointer */
    uint32_t eip;                 /* Saved instruction pointer */

    uint32_t stack[STACK_SIZE / 4];

    struct pcb *next;             /* Ready queue link */
} pcb_t;

/* Process management interface */
void process_init(void);
pcb_t *process_create(void (*entry)(void));
void process_yield(void);
void process_exit(void);
void scheduler_tick(void);
void scheduler_add(pcb_t *process);

/* Return the PCB at the given process-table index. */
const pcb_t *process_get(uint32_t index);

#endif
