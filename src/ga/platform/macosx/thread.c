
/*
    gaudiamus

 */

#include "config.h"

#if MACOSX

#include <ga/thread.h>
#include <ga/util.h>
#include <ga/alloc.h>

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


struct ga_thread {
    pthread_t native;
    char *name;
};

struct ga_mutex {
    pthread_mutex_t native;
};

static pthread_t main_thread = NULL;

// --------------------------------------------------------------------------------

void ga_thread_initialize()
{
    main_thread = pthread_self();
}

void ga_thread_terminate()
{
    main_thread = NULL;
}

// --------------------------------------------------------------------------------

ga_thread* ga_thread_create(ga_thread_func func, void* data)
{
    return ga_thread_create_named(func, data, NULL);
}

ga_thread* ga_thread_create_named(ga_thread_func func, void* data, const char *name)
{
    ga_thread* thread = ga_new(ga_thread);
    thread->name = name ? strdup(name) : NULL;

    int result = pthread_create(&thread->native, NULL, func, data);

    if (result != 0) {
        fatal_error("Couldn't spawn thread! %d", result);
    }

    if (name) {
        printf("New thread %p '%s'\n", &thread->native, name);
    }

    return thread;
}

void ga_thread_sleep(unsigned int ms)
{
    usleep(ms * 1000);
}

void* ga_thread_join(ga_thread *thread)
{
    void* return_value;
    int result = pthread_join(thread->native, &return_value);

    if (result != 0) {
        fatal_error("pthread_join: %d for thread %s", result, thread->name ? thread->name : "<unnamed>");
    }
    if (thread->name) {
        printf("Joined thread %p '%s', return value: %p\n", thread->native, thread->name, return_value);
        free(thread->name);
    }
    ga_free(thread);
    return return_value;
}

void ga_thread_detach(ga_thread *thread)
{
    int result = pthread_detach(thread->native);

    if (result != 0) {
        fatal_error("pthread_detach: %d for thread %s", result, thread->name ? thread->name : "<unnamed>");
    }
    if (thread->name) {
        printf("Detached thread %p '%s'\n", thread->native, thread->name);
        free(thread->name);
    }

    ga_free(thread);
}

bool ga_thread_is_main(ga_thread *thread)
{
    assert(main_thread && "Module not initialized");
    return thread->native == main_thread;
}

bool ga_thread_is_current(ga_thread *thread)
{
    return thread->native == pthread_self();
}


// --------------------------------------------------------------------------------

// ga_mutex_t ga_thread_create_mutex()
// {
//     ga_mutex_t mutex = ga_new(ga_mutex_t);

//     int result = pthread_mutex_init(&mutex->native, NULL);

//     if (result != 0) {
//         fatal_error("create_mutex %d", result);
//     }

//     return mutex;
// }

// void ga_thread_destroy_mutex(ga_thread_mutex_t mutex)
// {
//     int result = pthread_mutex_destroy(&mutex->native);
//     ga_delete(mutex);

//     if (result != 0) {
//         ga_thread_fatal("destroy_mutex", result);
//     }
// }

// bool ga_thread_lock(ga_thread_mutex_t mutex)
// {
//     int result = pthread_mutex_lock(&mutex->native);

//     if (result == 0) {
//         return true;
//     } else {
//         ga_thread_fatal("unlock", result);
//         assert(false && "Not reached");
//     }
// }

// bool ga_thread_try_lock(ga_thread_mutex_t mutex)
// {
//     int result = pthread_mutex_trylock(&mutex->native);

//     switch (result) {
//     case 0:
//         return true;

//     case EBUSY:
//         return false;

//     default:
//         ga_thread_fatal("try_lock", result);
//         assert(false && "Not reached");
//     }
// }

// bool ga_thread_unlock(ga_thread_mutex_t mutex)
// {
//     int result = pthread_mutex_unlock(&mutex->native);

//     if (result == 0) {
//         return true;
//     } else {
//         ga_thread_fatal("unlock", result);
//         assert(false && "Not reached");
//     }
// }

#endif
