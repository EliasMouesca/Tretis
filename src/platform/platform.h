#ifndef PLATFORM_H
#define PLATFORM_H

#include <stddef.h>

void ensureParentDir(const char* path);
void makeDefaultRuntimePath(char* buffer, size_t size, const char* filename);

#endif
