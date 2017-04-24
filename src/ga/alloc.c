#include "ga/alloc.h"

// #include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "ga/util.h"
#if GA_DEBUG
#include <stdatomic.h>
#endif

#if GA_DEBUG
static atomic_size_t gBytesCurAlloc   = 0;
static atomic_size_t gBytesMaxAlloc   = 0;
static atomic_size_t gBytesTotAlloc   = 0;
static atomic_size_t gRegionsCurAlloc = 0;
static atomic_size_t gRegionsMaxAlloc = 0;
static atomic_size_t gRegionsTotAlloc = 0;
#else
static size_t gBytesTotAlloc   = 0;
static size_t gRegionsCurAlloc = 0;
#endif

// static fa_error_severity_t  gLogLevel     = info;




void* ga_malloc(size_t size)
{
#if GA_DEBUG
    if (size == 0) {
        printf("Warning: ga_malloc(0), returning NULL\n");
        return NULL;
    }
    if (size >= 2147483648) fatal_error("Request for allocation of %zu bytes of memory, limit is 2 GB", size);
    void *rawptr = malloc(size+sizeof(size_t));
    if (!rawptr) fatal_error("Could not allocate %zu bytes of memory", size);
    atomic_fetch_add(&gBytesTotAlloc, size);
    atomic_fetch_add(&gBytesCurAlloc, size);
    if (gBytesCurAlloc > gBytesMaxAlloc) gBytesMaxAlloc = gBytesCurAlloc; // Don't care with atomic for max
    atomic_fetch_add(&gRegionsTotAlloc, 1);
    atomic_fetch_add(&gRegionsCurAlloc, 1);
    if (gRegionsCurAlloc > gRegionsMaxAlloc) gRegionsMaxAlloc = gRegionsCurAlloc;
    *(size_t*)rawptr = size;
    return rawptr+sizeof(size_t);
#else
    gBytesTotAlloc += size;
    gRegionsCurAlloc += 1;
    return malloc(size);
#endif
}

void* ga_calloc(size_t count, size_t size)
{
#if GA_DEBUG
    void* ptr = ga_malloc(count * size);
    memset(ptr, 0, count * size);
    return ptr;
#else
    gBytesTotAlloc += size;
    gRegionsCurAlloc += 1;
    return calloc(count, size);
#endif
}

void* ga_realloc(void *ptr, size_t size)
{
#if GA_DEBUG
    assert(ptr && "Trying to realloc NULL pointer");
    void *rawptr = ptr - sizeof(size_t);
    size_t old_size = *(size_t*)rawptr;
    rawptr = realloc(rawptr, size + sizeof(size_t));
    if (!rawptr) fatal_error("Could not reallocate %zu bytes of memory", size);
    atomic_fetch_add(&gBytesTotAlloc, size - old_size);
    atomic_fetch_add(&gBytesCurAlloc, size - old_size);
    if (gBytesCurAlloc > gBytesMaxAlloc) gBytesMaxAlloc = gBytesCurAlloc; // Don't care with atomic for max
    *(size_t*)rawptr = size;
    return rawptr+sizeof(size_t);
#else
    return realloc(ptr, size);
#endif
}

void ga_free(void *ptr)
{
#if GA_DEBUG
    assert(ptr && "Trying to free NULL pointer");
    void *rawptr = ptr - sizeof(size_t);
    size_t size = *(size_t*)rawptr;
    atomic_fetch_add(&gBytesCurAlloc, -size);
    atomic_fetch_add(&gRegionsCurAlloc, -1);
    free(rawptr);
#else
    gRegionsCurAlloc--;
    free(ptr);
#endif
}


void ga_print_alloc_info()
{
#if GA_DEBUG
    printf("Currently allocated:   %zu bytes, %zu regions\n", gBytesCurAlloc, gRegionsCurAlloc);
    printf("Peak:                  %zu bytes, %zu regions\n",  gBytesMaxAlloc, gRegionsMaxAlloc);
    printf("Total allocated:       %zu bytes, %zu regions\n", gBytesTotAlloc, gRegionsTotAlloc);
#else
    printf("%zu regions currently allocated, %zu bytes in total\n", gRegionsCurAlloc, gBytesTotAlloc);
    printf("ga_print_alloc_info: not compiled with DEBUG\n");
#endif
}
