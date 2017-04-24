
#ifndef _GA_UTIL_MINMAX
#define _GA_UTIL_MINMAX

#include <stddef.h>
#include <stdint.h>

#define min(x, y) _Generic((x), \
    double: min_double, \
    float: _Generic((y), \
        double: min_double, \
        default: min_float), \
    size_t: _Generic((y), \
        size_t: min_size_t, \
        float: min_float, \
        double: min_double, \
        uint32_t: min_size_t, \
        uint64_t: min_size_t, \
        default: min_int), \
    uint8_t: _Generic((y), \
        size_t: min_size_t, \
        float: min_float, \
        double: min_double, \
        uint8_t: min_uint8_t, \
        uint32_t: min_size_t, \
        uint64_t: min_size_t, \
        default: min_int), \
    default: _Generic((y), \
        double: min_double, \
        float: min_float, \
        default: min_int) \
     )(x, y)

#define max(x, y) _Generic((x), \
    double: max_double, \
    float: _Generic((y), \
        double: max_double, \
        default: max_float), \
    size_t: _Generic((y), \
        size_t: max_size_t, \
        float: max_float, \
        double: max_double, \
        uint32_t: max_size_t, \
        uint64_t: max_size_t, \
        default: max_int), \
    uint8_t: _Generic((y), \
        size_t: max_size_t, \
        float: max_float, \
        double: max_double, \
        uint8_t: max_uint8_t, \
        uint32_t: max_size_t, \
        uint64_t: max_size_t, \
        default: max_int), \
    default: _Generic((y), \
        double: max_double, \
        float: max_float, \
        default: max_int) \
     )(x, y)

#define GENERATE_MIN(type) inline static type min_ ## type (type a, type b) { return (a < b) ? a : b; }
#define GENERATE_MAX(type) inline static type max_ ## type (type a, type b) { return (a > b) ? a : b; }

GENERATE_MIN(double);
GENERATE_MIN(float);
GENERATE_MIN(size_t);
GENERATE_MIN(int);
GENERATE_MIN(uint8_t);

GENERATE_MAX(double);
GENERATE_MAX(float);
GENERATE_MAX(size_t);
GENERATE_MAX(int);
GENERATE_MAX(uint8_t);

#undef GENERATE_MIN
#undef GENERATE_MAX

#endif

