#include <stdatomic.h>
#include <ga/util.h>
#include "config.h"

int ga_semaphore_acquire(unsigned int initial_value);
void ga_semaphore_release(int id);
static inline unsigned int ga_semaphore_get(int id);
static inline void ga_semaphore_set(int id, unsigned int value);

// "Private" stuff below
// (Must be present in the header file to enable inlining)

typedef struct ga_semaphore
{
    atomic_uint status;
    atomic_uint used;
    char pad[CACHELINE_SIZE - (sizeof(atomic_uint) * 2)];
} ga_semaphore;

static inline unsigned int ga_semaphore_get(int id)
{
    extern ga_semaphore ga_semaphores[];
    return atomic_load_explicit(&ga_semaphores[id].status, memory_order_acquire);
}

static inline void ga_semaphore_set(int id, unsigned int value)
{
    extern ga_semaphore ga_semaphores[];
    atomic_store_explicit(&ga_semaphores[id].status, value, memory_order_release);
}

