#include "ga/semaphore.h"

#define MAX_SEMAPHORES 32

ga_semaphore ga_semaphores[MAX_SEMAPHORES] = {};

int ga_semaphore_acquire(unsigned int initial_value)
{
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (atomic_load_explicit(&ga_semaphores[i].used, memory_order_relaxed)) continue;
        bool b = 0;
        if (atomic_compare_exchange_strong(&ga_semaphores[i].used, &b, 1)) {
            ga_semaphore_set(i, initial_value);
            return i;
        }
    }
    fatal_error("No free semaphores");
    // return 0; // avoid compiler warning
}

void ga_semaphore_release(int id)
{
    atomic_store(&ga_semaphores[id].used, 0);
}



