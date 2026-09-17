#include "process.h"
#include "thread.h"

/*
 * Simple Round-Robin Scheduler
 *
 * The ready queue is implemented as a linked list of PCBs.
 */

static pcb_t *ready_head = 0;
static pcb_t *ready_tail = 0;
static pcb_t *current_process = 0;

/*
 * Tracks which kind of context currently owns the CPU.
 * 0 = process/kernel context
 * 1 = thread context
 */
static int running_thread = 0;
static int prefer_thread = 1;

/*
 * PCB used to preserve the original kernel/shell execution context.
 * PID 0 is reserved for the kernel context.
 */
static pcb_t kernel_context;
static int kernel_context_saved = 0;


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

/*
 * Preemptive Round-Robin context switch.
 * current_esp points to the interrupt frame saved by irq0_stub.
 * Returns the ESP of the process that should run next.
 */
uint32_t scheduler_switch(uint32_t current_esp)
{
    pcb_t *next;
    thread_t *next_thread;


    /*
     * On the first timer interrupt, preserve the kernel/shell
     * context so that Round-Robin can return to it later.
     */
    if (!kernel_context_saved) {
        kernel_context.pid = 0;
        kernel_context.state = READY;
        kernel_context.esp = current_esp;
        kernel_context.eip = 0;
        kernel_context.next = 0;

        kernel_context_saved = 1;
        enqueue(&kernel_context);
    }
    else if (running_thread) {
        /*
         * The interrupted context belongs to a Stage 2 thread.
         * Save its real CPU stack pointer before selecting another context.
         */
        thread_preempt_current(current_esp);
	running_thread = 0;
    }
    else if (current_process != 0 &&
             current_process->state == RUNNING) {

        /* Save the interrupted process/kernel context. */
        current_process->esp = current_esp;
        current_process->state = READY;
        enqueue(current_process);
    }

/* Give a ready kernel thread a turn on the CPU. */
if (prefer_thread && thread_ready_available()) {
    next_thread = thread_schedule_next();

    if (next_thread != 0) {
        running_thread = 1;
	prefer_thread = 0;
        current_process = 0;
        return next_thread->esp;
    }
}

    /* Select the next context in Round-Robin order. */
    next = dequeue();

    if (next == 0) {
    if (thread_ready_available()) {
        next_thread = thread_schedule_next();

        if (next_thread != 0) {
            running_thread = 1;
            prefer_thread = 0;
            current_process = 0;
            return next_thread->esp;
        }
    }

    return current_esp;
}

    next->state = RUNNING;
    current_process = next;
    running_thread = 0;
    prefer_thread = 1;

    return next->esp;
}
