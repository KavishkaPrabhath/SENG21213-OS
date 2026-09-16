#include "mutex.h"
#include "thread.h"

void mutex_init(mutex_t *mutex)
{
    if (mutex == 0)
        return;

    mutex->locked = 0;
}

void mutex_lock(mutex_t *mutex)
{
    if (mutex == 0)
        return;

    while (__sync_lock_test_and_set(&mutex->locked, 1)) {
        thread_yield();
    }
}

void mutex_unlock(mutex_t *mutex)
{
    if (mutex == 0)
        return;

    __sync_lock_release(&mutex->locked);
}
