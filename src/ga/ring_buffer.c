#include "ga/ring_buffer.h"

#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "ga/util.h"
#include "ga/alloc.h"

struct ga_ring_buffer {
    size_t                   size;                  //  Size (immutable)
    size_t                   first, last;           //  Next read or write, always < size
    atomic_size_t            count;                 //  Number of bytes available for reading
    void                     *data;                 //  The actual data
    ga_ring_buffer_callback  error_callback;        //
    void                     *error_callback_data;
};


ga_ring_buffer* ga_ring_buffer_create(size_t size)
{
    ga_ring_buffer *ring_buffer = ga_newc(ga_ring_buffer);
    ring_buffer->size = size;
    ring_buffer->data = ga_malloc(size);
    return ring_buffer;
}

void ga_ring_buffer_destroy(ga_ring_buffer *ring_buffer)
{
    ga_free(ring_buffer->data);
    ga_free(ring_buffer);
}

void ga_ring_buffer_set_error_callback(ga_ring_buffer *ring_buffer, ga_ring_buffer_callback callback, void *data)
{
    ring_buffer->error_callback = callback;
    ring_buffer->error_callback_data = data;
}

size_t ga_ring_buffer_can_read(ga_ring_buffer *ring_buffer)
{
    return atomic_load_explicit(&ring_buffer->count, memory_order_acquire);
}

size_t ga_ring_buffer_can_write(ga_ring_buffer *ring_buffer)
{
    return ring_buffer->size - atomic_load_explicit(&ring_buffer->count, memory_order_acquire);
}

static inline void internal_write(ga_ring_buffer *ring_buffer, size_t bytes, void *data)
{
    if (ring_buffer->last + bytes < ring_buffer->size) {
        // New data fits after the write position
        // printf("Single write\n");
        memcpy(ring_buffer->data + ring_buffer->last, data, bytes);
        ring_buffer->last = (ring_buffer->last + bytes) % ring_buffer->size;
    } else {
        // Doesn't fit, so write as much as we can, and then wrap around
        // printf("Split write\n");
        size_t to_end = ring_buffer->size - ring_buffer->last;
        memcpy(ring_buffer->data + ring_buffer->last, data, to_end);
        size_t bytes_left = bytes - to_end;
        memcpy(ring_buffer->data, data + to_end, bytes_left);
        ring_buffer->last = bytes_left;
    }
    atomic_fetch_add_explicit(&ring_buffer->count, bytes, memory_order_release);
}

static inline void internal_read(ga_ring_buffer *ring_buffer, size_t bytes, void *data)
{
    if (ring_buffer->first + bytes < ring_buffer->size) {
        // printf("Single read\n");
        memcpy(data, ring_buffer->data + ring_buffer->first, bytes);
        ring_buffer->first = (ring_buffer->first + bytes) % ring_buffer->size;
    } else {
        // printf("Split read\n");
        size_t to_end = ring_buffer->size - ring_buffer->first;
        memcpy(data, ring_buffer->data + ring_buffer->first, to_end);
        size_t bytes_left = bytes - to_end;
        memcpy(ring_buffer->data, data + to_end, bytes_left);
        ring_buffer->first = bytes_left;
    }
    atomic_fetch_sub_explicit(&ring_buffer->count, bytes, memory_order_release);
}

size_t ga_ring_buffer_write(ga_ring_buffer *ring_buffer, size_t bytes, void *data)
{
    assert(bytes <= ring_buffer->size);
    if (!bytes) return 0;
    size_t can_write = ga_ring_buffer_can_write(ring_buffer);
    if (can_write < bytes) {
        bytes = can_write;
        if (ring_buffer->error_callback) {
            ring_buffer->error_callback(ring_buffer, GA_ERROR_OVERFLOW, ring_buffer->error_callback_data);
        }
        if (!bytes) return 0;
    }
    internal_write(ring_buffer, bytes, data);
    return bytes;
}

size_t ga_ring_buffer_write_atomic(ga_ring_buffer *ring_buffer, size_t bytes, void *data)
{
    assert(bytes <= ring_buffer->size);
    if (!bytes) return 0;
    if (bytes > ga_ring_buffer_can_write(ring_buffer)) {
        if (ring_buffer->error_callback) {
            ring_buffer->error_callback(ring_buffer, GA_ERROR_OVERFLOW, ring_buffer->error_callback_data);
        }
        return 0;
    }
    internal_write(ring_buffer, bytes, data);
    return bytes;
}

size_t ga_ring_buffer_read(ga_ring_buffer *ring_buffer, size_t bytes, void *data)
{
    assert(bytes <= ring_buffer->size);
    if (!bytes) return 0;
    size_t can_read = ga_ring_buffer_can_read(ring_buffer);
    if (can_read < bytes) {
        bytes = can_read;
        if (ring_buffer->error_callback) {
            ring_buffer->error_callback(ring_buffer, GA_ERROR_UNDERFLOW, ring_buffer->error_callback_data);
        }
        if (!bytes) return 0;
    }
    internal_read(ring_buffer, bytes, data);
    return bytes;
}

size_t ga_ring_buffer_read_atomic(ga_ring_buffer *ring_buffer, size_t bytes, void *data)
{
    assert(bytes <= ring_buffer->size);
    if (!bytes) return 0;
    if (bytes > ga_ring_buffer_can_read(ring_buffer)) {
        if (ring_buffer->error_callback) {
            ring_buffer->error_callback(ring_buffer, GA_ERROR_UNDERFLOW, ring_buffer->error_callback_data);
        }
        return 0;
    }
    internal_read(ring_buffer, bytes, data);
    return bytes;
}


static char* char_repeat(int n, char c) {
    char *dest = malloc(n+1);
    memset(dest, c, n);
    dest[n] = '\0';
    return dest;
}

void debug_ring_buffer(ga_ring_buffer *ring_buffer)
{
    printf("RB: ");
    for (int i = 0; i < ring_buffer->size; i++) {
        printf("%02x ", *(uint8_t*)(ring_buffer->data + i));
    }
    printf("\n    %sR", char_repeat(ring_buffer->first * 3, ' '));
    printf("\n    %sW", char_repeat(ring_buffer->last * 3, ' '));

    printf("\n");
}

