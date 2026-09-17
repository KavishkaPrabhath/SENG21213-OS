#include "mutex.h"

static void mutex_enqueue_waiter(mutex_t *mutex, thread_t *thread)
{
    thread->next = 0;

    if (mutex->wait_tail == 0) {
        mutex->wait_head = thread;
        mutex->wait_tail = thread;
    } else {
        mutex->wait_tail->next = thread;
        mutex->wait_tail = thread;
    }
}

static thread_t *mutex_dequeue_waiter(mutex_t *mutex)
{
    thread_t *thread = mutex->wait_head;

    if (thread == 0)
        return 0;

    mutex->wait_head = thread->next;

    if (mutex->wait_head == 0)
        mutex->wait_tail = 0;

    thread->next = 0;
    return thread;
}

void mutex_init(mutex_t *mutex)
{
    if (mutex == 0)
        return;

    mutex->locked = 0;
    mutex->wait_head = 0;
    mutex->wait_tail = 0;
}

void mutex_lock(mutex_t *mutex)
{
    thread_t *current;

    if (mutex == 0)
        return;

    while (__sync_lock_test_and_set(&mutex->locked, 1)) {
        current = thread_current();

        if (current != 0 && current->state == THREAD_RUNNING) {
            mutex_enqueue_waiter(mutex, current);
            thread_block_current();

            while (current->state == THREAD_BLOCKED)
                thread_yield();
        } else {
            thread_yield();
        }
    }
}

void mutex_unlock(mutex_t *mutex)
{
    thread_t *waiter;

    if (mutex == 0)
        return;

    __sync_lock_release(&mutex->locked);

    waiter = mutex_dequeue_waiter(mutex);

    if (waiter != 0)
        thread_wake(waiter);
}
