#include "platform.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int isSeparator(char c) {
    return c == '/';
}

void ensureParentDir(const char* path) {
    char buffer[512];
    char* slash = NULL;
    size_t start = 1;

    if (strlen(path) >= sizeof(buffer))
        return;

    snprintf(buffer, sizeof(buffer), "%s", path);

    for (char* p = buffer; *p; p++)
        if (isSeparator(*p))
            slash = p;

    if (slash == NULL)
        return;

    *slash = '\0';

    for (char* p = buffer + start; *p; p++) {
        if (!isSeparator(*p))
            continue;

        char separator = *p;
        *p = '\0';
        if (mkdir(buffer, 0700) != 0 && errno != EEXIST)
            return;
        *p = separator;
    }

    mkdir(buffer, 0700);
}

void makeDefaultRuntimePath(char* buffer, size_t size, const char* filename) {
    const char* home = getenv("HOME");
    if (home != NULL && home[0] != '\0') {
        snprintf(buffer, size, "%s/.config/tretis/%s", home, filename);
        return;
    }

    snprintf(buffer, size, ".tretis_%s", filename);
}
