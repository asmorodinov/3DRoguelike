#pragma once

#include <cstdlib>
#include <cstdio>

#define STR(x) #x
#define LOG_ASSERT(x)                                                                          \
    do {                                                                                       \
        if (!(x)) {                                                                            \
            printf("Assertion failed: (%s), file %s, line %d.\n", STR(x), __FILE__, __LINE__); \
            std::exit(1);                                                                      \
        }                                                                                      \
    } while (0)
