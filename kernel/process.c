#include "process.h"

/*
 * Process table.
 * Stage 1 uses a fixed-size table because dynamic memory
 * allocation is not available yet.
 */
static pcb_t process_table[MAX_PROCESSES];

static uint32_t next_pid = 1;

/* Initialise all PCB entries as unused/terminated. */
void process_init(void)
{
    uint32_t i;

    for (i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = TERMINATED;
        process_table[i].esp = 0;
        process_table[i].eip = 0;
        process_table[i].next = 0;
    }

    next_pid = 1;
}

/*
 * Create a new process and initialise its PCB.
 */
pcb_t *process_create(void (*entry)(void))
{
    uint32_t i;
    pcb_t *process = 0;

    /* Find a free PCB slot. */
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == TERMINATED) {
            process = &process_table[i];
            break;
        }
    }

    /* Process table is full. */
    if (process == 0) {
        return 0;
    }

    process->pid = next_pid++;
    process->state = READY;
    process->eip = (uint32_t)entry;

    /*
     * Stack grows downward, so ESP initially points
     * to the top of this process's private stack.
     */
    process->esp =
        (uint32_t)&process->stack[STACK_SIZE / 4];

    process->next = 0;

    scheduler_add(process);

    return process;
}

/*
 * Return a read-only pointer to a PCB in the process table.
 * Returns 0 if the index is outside the table.
 */
const pcb_t *process_get(uint32_t index)
{
    if (index >= MAX_PROCESSES) {
        return 0;
    }

    return &process_table[index];
}
