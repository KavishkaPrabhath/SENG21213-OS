#include "semaphore.h"

static void semaphore_enqueue_waiter(semaphore_t *sem, thread_t *thread)
{
    thread->next = 0;

    if (sem->wait_tail == 0) {
        sem->wait_head = thread;
        sem->wait_tail = thread;
    } else {
        sem->wait_tail->next = thread;
        sem->wait_tail = thread;
    }
}

static thread_t *semaphore_dequeue_waiter(semaphore_t *sem)
{
    thread_t *thread = sem->wait_head;

    if (thread == 0)
        return 0;

    sem->wait_head = thread->next;

    if (sem->wait_head == 0)
        sem->wait_tail = 0;

    thread->next = 0;
    return thread;
}

void semaphore_init(semaphore_t *sem, int32_t initial_count)
{
    if (sem == 0)
        return;

    if (initial_count < 0)
        initial_count = 0;

    sem->count = initial_count;
    sem->wait_head = 0;
    sem->wait_tail = 0;
}

void semaphore_wait(semaphore_t *sem)
{
    thread_t *current;

    if (sem == 0)
        return;

    while (1) {
        if (sem->count > 0) {
            int32_t old_count = sem->count;

            if (__sync_bool_compare_and_swap(
                    &sem->count, old_count, old_count - 1))
                return;
        }

        current = thread_current();

        if (current != 0 && current->state == THREAD_RUNNING) {
            semaphore_enqueue_waiter(sem, current);
            thread_block_current();

            while (current->state == THREAD_BLOCKED)
                thread_yield();
        } else {
            thread_yield();
        }
    }
}

void semaphore_signal(semaphore_t *sem)
{
    thread_t *waiter;

    if (sem == 0)
        return;

    __sync_fetch_and_add(&sem->count, 1);

    waiter = semaphore_dequeue_waiter(sem);

    if (waiter != 0)
        thread_wake(waiter);
}
