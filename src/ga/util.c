#include "ga/util.h"
#include "ga/alloc.h"

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifndef HAVE_STRDUP
char *strdup(const char *s) {
    size_t size = strlen(s) + 1;
    char *p = malloc(size);
    if (p) {
        memcpy(p, s, size);
    }
    return p;
}
#endif

char *ga_sprintf(const char *format, ...)
{
    int n;
    int size = 100;     /* Guess we need no more than 100 bytes */
    char *p, *np;
    va_list ap;

   if ((p = ga_malloc(size)) == NULL)
        return NULL;

    while (1) {

        /* Try to print in the allocated space */
        va_start(ap, format);
        n = vsnprintf(p, size, format, ap);
        va_end(ap);

        /* Check error code */
        if (n < 0) return NULL;

        /* If that worked, return the string */
        if (n < size) return p;

        /* Else try again with more space */
        size = n + 1;       /* Precisely what is needed */

        if ((np = ga_realloc (p, size)) == NULL) {
            ga_free(p);
            return NULL;
        } else {
            p = np;
        }
    }
}

#if GA_DEBUG
void _fatal_error(const char *msg, const char *func, const char *file, int line)
{
    if (msg) {
        printf("FATAL ERROR in %s at %s:%d: %s\n", func, file, line, msg);
    } else {
        printf("FATAL ERROR in %s at %s:%d\n", func, file, line);
    }
    assert(false && "Fatal error");
}
#else
void _fatal_error(const char *msg, const char *func)
{
    if (msg) {
        printf("FATAL ERROR in %s: %s\n", func, msg);
    } else {
        printf("FATAL ERROR in %s\n", func);
    }
    exit(1);
}
#endif

