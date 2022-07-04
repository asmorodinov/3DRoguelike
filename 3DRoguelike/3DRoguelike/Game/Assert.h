#pragma once

#include <cstdlib>
#include <cstdio>

#define STR(x) #x
#define LOG_ASSERT(x)                                                                      \
    if (!(x)) {                                                                            \
        printf("Assertion failed: (%s), file %s, line %d.\n", STR(x), __FILE__, __LINE__); \
        std::exit(1);                                                                      \
    }
