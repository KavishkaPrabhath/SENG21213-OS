#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "../include/types.h"
#include "thread.h"

typedef struct {
    volatile int32_t count;
    thread_t *wait_head;
    thread_t *wait_tail;
} semaphore_t;

void semaphore_init(semaphore_t *sem, int32_t initial_count);
void semaphore_wait(semaphore_t *sem);
void semaphore_signal(semaphore_t *sem);

#endif
