#include "platform.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define PATH_SEPARATOR '\\'
#define mkdirPlatform(path) _mkdir(path)
#else
#define PATH_SEPARATOR '/'
#define mkdirPlatform(path) mkdir(path, 0700)
#endif

static int isSeparator(char c) {
    return c == '/' || c == '\\';
}

void ensureParentDir(const char* path) {
    char buffer[512];
    char* slash = NULL;
    size_t start = 1;

    if (strlen(path) >= sizeof(buffer))
        return;

    strcpy(buffer, path);

    for (char* p = buffer; *p; p++)
        if (isSeparator(*p))
            slash = p;

    if (slash == NULL)
        return;

    *slash = '\0';

#ifdef _WIN32
    if (strlen(buffer) >= 3 && buffer[1] == ':' && isSeparator(buffer[2]))
        start = 3;
#endif

    for (char* p = buffer + start; *p; p++) {
        if (!isSeparator(*p))
            continue;

        char separator = *p;
        *p = '\0';
        if (mkdirPlatform(buffer) != 0 && errno != EEXIST)
            return;
        *p = separator;
    }

    mkdirPlatform(buffer);
}

void makeDefaultRuntimePath(char* buffer, size_t size, const char* filename) {
#ifdef _WIN32
    const char* base = getenv("APPDATA");
    if (base == NULL || base[0] == '\0')
        base = getenv("USERPROFILE");

    if (base != NULL && base[0] != '\0') {
        snprintf(buffer, size, "%s%ctretis%c%s", base, PATH_SEPARATOR, PATH_SEPARATOR, filename);
        return;
    }
#else
    const char* home = getenv("HOME");
    if (home != NULL && home[0] != '\0') {
        snprintf(buffer, size, "%s/.config/tretis/%s", home, filename);
        return;
    }
#endif

    snprintf(buffer, size, ".tretis_%s", filename);
}
