#pragma once

#include <stdint.h>

typedef struct {
    const char *name;
    const unsigned char *start;
    const unsigned int len;
} library;

extern library* g_libs;

library* load_libraries(const char* dir);
void free_libraries(library* libs);
