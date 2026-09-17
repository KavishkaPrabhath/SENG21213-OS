#ifndef MUTEX_H
#define MUTEX_H

#include "../include/types.h"
#include "thread.h"

typedef struct {
    volatile uint32_t locked;
    thread_t *wait_head;
    thread_t *wait_tail;
} mutex_t;

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

#endif
