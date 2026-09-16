#include "process.h"

/*
 * Simple Round-Robin Scheduler
 *
 * The ready queue is implemented as a linked list of PCBs.
 */

static pcb_t *ready_head = 0;
static pcb_t *ready_tail = 0;
static pcb_t *current_process = 0;


/* Add a process to the end of the ready queue */
static void enqueue(pcb_t *process)
{
    if (process == 0)
        return;

    process->next = 0;

    if (ready_tail == 0) {
        ready_head = process;
        ready_tail = process;
    } else {
        ready_tail->next = process;
        ready_tail = process;
    }
}

/*
 * Add a newly created process to the ready queue.
 */
void scheduler_add(pcb_t *process)
{
    if (process == 0)
        return;

    process->state = READY;
    enqueue(process);
}

/* Remove and return the first process in the ready queue */
static pcb_t *dequeue(void)
{
    pcb_t *process;

    if (ready_head == 0)
        return 0;

    process = ready_head;
    ready_head = ready_head->next;

    if (ready_head == 0)
        ready_tail = 0;

    process->next = 0;

    return process;
}


/*
 * Called when the current process voluntarily gives up the CPU.
 *
 * Stage 1 implements the scheduling logic.
 * Actual low-level context switching can be connected later.
 */
void process_yield(void)
{
    pcb_t *next;

    if (current_process != 0 &&
        current_process->state == RUNNING) {

        current_process->state = READY;
        enqueue(current_process);
    }

    next = dequeue();

    if (next == 0) {
        current_process = 0;
        return;
    }

    next->state = RUNNING;
    current_process = next;
}


/*
 * Terminate the currently running process.
 */
void process_exit(void)
{
    if (current_process == 0)
        return;

    current_process->state = TERMINATED;
    current_process = 0;

    process_yield();
}


/*
 * Scheduler timer hook.
 *
 * Lecture 10 can call this function from the timer IRQ
 * to provide preemptive round-robin scheduling.
 */
void scheduler_tick(void)
{
    process_yield();
}
