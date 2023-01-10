#pragma once

#include <stdint.h>

typedef struct _library{
    char *name;
    unsigned char *start;
    unsigned int length;
} library;

extern library* g_libs;

library* load_libraries(const char* working, const char* subdir);
void free_libraries(library* libs);
