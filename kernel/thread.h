#ifndef THREAD_H
#define THREAD_H

#include "../include/types.h"

#define MAX_THREADS 16
#define THREAD_STACK_SIZE 2048

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_TERMINATED
} thread_state_t;

typedef struct thread {
    uint32_t tid;
    uint32_t owner_pid;
    thread_state_t state;

    void (*entry)(void);

    uint32_t esp;
    uint32_t stack[THREAD_STACK_SIZE / 4];

    struct thread *next;
} thread_t;

void thread_init(void);
thread_t *thread_create(uint32_t owner_pid, void (*entry)(void));
void thread_yield(void);
void thread_exit(void);

/* Scheduler helpers for real thread context switching. */
thread_t *thread_schedule_next(void);
int thread_ready_available(void);
thread_t *thread_current(void);
void thread_preempt_current(uint32_t esp);
void thread_block_current(void);
void thread_wake(thread_t *thread);

const thread_t *thread_get(uint32_t index);
const char *thread_state_name(thread_state_t state);

#endif
