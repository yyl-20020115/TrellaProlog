#pragma once

#include <stdint.h>

typedef struct {
    char *name;
    unsigned char *start;
    unsigned int length;
} library;

extern library* g_libs;

library* load_libraries(const char* dir);
void free_libraries(library* libs);
