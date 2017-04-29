#ifndef _GA_UTIL
#define _GA_UTIL

#include "ga/util/vararg.h"
#include "ga/util/minmax.h"

#if GA_DEBUG
#define fatal_error0() _fatal_error(NULL, __func__, __FILE__, __LINE__);
#define fatal_error1(msg) _fatal_error(msg, __func__, __FILE__, __LINE__);
#define fatal_error2(msg, a) _fatal_error(ga_sprintf(msg, a), __func__, __FILE__, __LINE__);
#define fatal_error3(msg, a, b) _fatal_error(ga_sprintf(msg, a, b), __func__, __FILE__, __LINE__);
#define fatal_error4(msg, a, b, c) _fatal_error(ga_sprintf(msg, a, b, c), __func__, __FILE__, __LINE__);
#else
#define fatal_error0() _fatal_error(NULL, __func__);
#define fatal_error1(msg) _fatal_error(msg, __func__);
#define fatal_error2(msg, a) _fatal_error(ga_sprintf(msg, a), __func__);
#define fatal_error3(msg, a, b) _fatal_error(ga_sprintf(msg, a, b), __func__);
#define fatal_error4(msg, a, b, c) _fatal_error(ga_sprintf(msg, a, b, c), __func__);
#endif
#define fatal_error(...) VARARG(fatal_error, __VA_ARGS__)

typedef enum { false, true } bool;

#ifndef HAVE_STRDUP
char *strdup(const char *s);
#endif
char *ga_sprintf(const char *, ...) __attribute__((format(printf,1,2)));

#if GA_DEBUG
void _fatal_error(const char *msg, const char *func, const char *file, int line) __attribute__((noreturn));
#else
void _fatal_error(const char *msg, const char *func) __attribute__((noreturn));
#endif

#endif
