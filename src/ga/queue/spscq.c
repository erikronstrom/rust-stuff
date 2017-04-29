#include "ga/queue/spscq.h"

#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ga/util.h"
#include "ga/alloc.h"
#include "ga/thread.h"

typedef ga_spscq_overflow_strategy overflow_strategy;

struct ga_spscq {
    size_t              size;                   //  Capacity (immutable)
    size_t              read_pos, write_pos;    //  Next read or write
    atomic_size_t       count;                  //  Number of items in the queue (always <= size)
    void                **data;                 //  The actual data buffer
    overflow_strategy   on_overflow;            //  What to do if buffer overflows
    size_t              overflows;              //  Number of overflows
    ga_spscq_callback   error_callback;         //
    void                *error_callback_data;
};


ga_spscq* ga_spscq_create(size_t capacity, ga_spscq_overflow_strategy on_overflow)
{
    assert(on_overflow != SPSCQ_OVERFLOW_GROW); // Not implemented
    ga_spscq *queue = ga_newc(ga_spscq);
    queue->size = capacity;
    queue->on_overflow = on_overflow;
    queue->data = ga_calloc(capacity, sizeof(void*));
    return queue;
}

void ga_spscq_destroy(ga_spscq *queue)
{
    ga_free(queue->data);
    ga_free(queue);
}

void ga_spscq_set_error_callback(ga_spscq *queue, ga_spscq_callback callback, void *data)
{
    queue->error_callback = callback;
    queue->error_callback_data = data;
}

size_t ga_spscq_can_push(ga_spscq *queue)
{
    return queue->size - atomic_load_explicit(&queue->count, memory_order_acquire);
}

size_t ga_spscq_can_pop(ga_spscq *queue)
{
    return atomic_load_explicit(&queue->count, memory_order_acquire);
}

bool ga_spscq_push(ga_spscq *queue, void *value)
{
    if (!ga_spscq_can_push(queue)) {
        queue->overflows++;
        switch(queue->on_overflow) {
        case SPSCQ_OVERFLOW_DISCARD:
            return false;
        case SPSCQ_OVERFLOW_BLOCK:
            while(!ga_spscq_can_push(queue)) {
                ga_thread_sleep(1);
            }
            break;
        case SPSCQ_OVERFLOW_GROW:
            assert(false && "Not implemented");
            break;
        case SPSCQ_OVERFLOW_ERROR:
            if (queue->error_callback) {
                queue->error_callback(queue, GA_ERROR_OVERFLOW, value, queue->error_callback_data);
            } else {
                assert(false && "SPSCQ_OVERFLOW_ERROR but no error callback set!");
            }
            return false;
        case SPSCQ_OVERFLOW_FATAL:
            fatal_error("Spscq overflow");
            break;
        }
    }
    queue->data[queue->write_pos] = value;
    queue->write_pos = (queue->write_pos + 1) % queue->size;
    atomic_fetch_add_explicit(&queue->count, 1, memory_order_release);
    return true;
}

void* ga_spscq_pop(ga_spscq *queue)
{
    if (!ga_spscq_can_pop(queue)) return NULL;
    void *value = queue->data[queue->read_pos];
    queue->read_pos = (queue->read_pos + 1) % queue->size;
    atomic_fetch_sub_explicit(&queue->count, 1, memory_order_release);
    return value;
}

void* ga_spscq_peek(ga_spscq *queue)
{
    if (!ga_spscq_can_pop(queue)) return NULL;
    return queue->data[queue->read_pos];
}

static char* char_repeat(int n, char c) {
  char *dest = malloc(n+1);
  memset(dest, c, n);
  dest[n] = '\0';
  return dest;
}

void debug_spscq(const ga_spscq *queue)
{
    printf("Q: ");
    for (int i = 0; i < queue->size; i++) {
        printf("%02x ", *(uint8_t*)(queue->data + i));
    }
    printf("\n   %sR", char_repeat(queue->read_pos * 3, ' '));
    printf("\n   %sW", char_repeat(queue->write_pos * 3, ' '));

    printf("\n");
}

