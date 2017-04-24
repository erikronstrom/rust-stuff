#ifndef _GA_ALLOC
#define _GA_ALLOC

#include <stdlib.h>

void* ga_malloc(size_t size);
void* ga_calloc(size_t count, size_t size);
void* ga_realloc(void *ptr, size_t size);
void ga_free(void *ptr);

void ga_print_alloc_info();

#define ga_new(type) ga_malloc(sizeof(type))
#define ga_newc(type) ga_calloc(sizeof(type), 1)

#endif
