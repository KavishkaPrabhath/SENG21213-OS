#include "thread.h"

static thread_t thread_table[MAX_THREADS];
static uint32_t thread_count = 0;
static uint32_t next_tid = 1;

static thread_t *ready_head = 0;
static thread_t *ready_tail = 0;
static thread_t *current_thread = 0;

static void enqueue(thread_t *thread)
{
    if (thread == 0)
        return;

    thread->next = 0;

    if (ready_tail == 0) {
        ready_head = thread;
        ready_tail = thread;
    } else {
        ready_tail->next = thread;
        ready_tail = thread;
    }
}

static thread_t *dequeue(void)
{
    thread_t *thread;

    if (ready_head == 0)
        return 0;

    thread = ready_head;
    ready_head = ready_head->next;

    if (ready_head == 0)
        ready_tail = 0;

    thread->next = 0;
    return thread;
}

void thread_init(void)
{
    uint32_t i;

    thread_count = 0;
    next_tid = 1;
    ready_head = 0;
    ready_tail = 0;
    current_thread = 0;

    for (i = 0; i < MAX_THREADS; i++) {
        thread_table[i].tid = 0;
        thread_table[i].owner_pid = 0;
        thread_table[i].state = THREAD_TERMINATED;
        thread_table[i].entry = 0;
        thread_table[i].esp = 0;
        thread_table[i].next = 0;
    }
}

thread_t *thread_create(uint32_t owner_pid, void (*entry)(void))
{
    thread_t *thread;

    if (thread_count >= MAX_THREADS || entry == 0)
        return 0;

    thread = &thread_table[thread_count];

    thread->tid = next_tid++;
    thread->owner_pid = owner_pid;
    thread->state = THREAD_READY;
    thread->entry = entry;
    thread->esp =
        (uint32_t)&thread->stack[THREAD_STACK_SIZE / 4];
    thread->next = 0;

    thread_count++;
    enqueue(thread);

    return thread;
}

void thread_yield(void)
{
    thread_t *next;

    if (current_thread != 0 &&
        current_thread->state == THREAD_RUNNING) {

        current_thread->state = THREAD_READY;
        enqueue(current_thread);
    }

    next = dequeue();

    if (next == 0) {
        current_thread = 0;
        return;
    }

    next->state = THREAD_RUNNING;
    current_thread = next;
}

void thread_exit(void)
{
    if (current_thread == 0)
        return;

    current_thread->state = THREAD_TERMINATED;
    current_thread = 0;

    thread_yield();
}

const thread_t *thread_get(uint32_t index)
{
    if (index >= thread_count)
        return 0;

    return &thread_table[index];
}

const char *thread_state_name(thread_state_t state)
{
    switch (state) {
        case THREAD_READY:
            return "READY";
        case THREAD_RUNNING:
            return "RUNNING";
        case THREAD_BLOCKED:
            return "BLOCKED";
        case THREAD_TERMINATED:
            return "TERMINATED";
        default:
            return "UNKNOWN";
    }
}
