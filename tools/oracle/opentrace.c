/* LD_PRELOAD open-trace: logs every host open() of a game file so we can see, in
 * order, which .BIN/.MRL/.MS3 the original opens on a given screen. Read-only. */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>

static void logname(const char *tag, const char *path) {
    if (!path) return;
    const char *b = strrchr(path, '/'); b = b ? b + 1 : path;
    /* only game asset extensions */
    const char *dot = strrchr(b, '.');
    if (!dot) return;
    if (strcasecmp(dot, ".BIN") && strcasecmp(dot, ".MRL") && strcasecmp(dot, ".MS3") &&
        strcasecmp(dot, ".KDV") && strcasecmp(dot, ".MAL") && strcasecmp(dot, ".SKY") &&
        strcasecmp(dot, ".KLC") && strcasecmp(dot, ".DTL") && strcasecmp(dot, ".RES"))
        return;
    int fd = 2;
    char line[256];
    int n = snprintf(line, sizeof line, "[OPEN:%s] %s\n", tag, b);
    write(fd, line, n);
}

int open64(const char *path, int flags, ...) {
    static int (*real)(const char *, int, ...) = 0;
    if (!real) real = dlsym(RTLD_NEXT, "open64");
    logname("open64", path);
    va_list ap; va_start(ap, flags); mode_t m = va_arg(ap, int); va_end(ap);
    return real(path, flags, m);
}
int open(const char *path, int flags, ...) {
    static int (*real)(const char *, int, ...) = 0;
    if (!real) real = dlsym(RTLD_NEXT, "open");
    logname("open", path);
    va_list ap; va_start(ap, flags); mode_t m = va_arg(ap, int); va_end(ap);
    return real(path, flags, m);
}
FILE *fopen64(const char *path, const char *mode) {
    static FILE *(*real)(const char *, const char *) = 0;
    if (!real) real = dlsym(RTLD_NEXT, "fopen64");
    logname("fopen64", path);
    return real(path, mode);
}
FILE *fopen(const char *path, const char *mode) {
    static FILE *(*real)(const char *, const char *) = 0;
    if (!real) real = dlsym(RTLD_NEXT, "fopen");
    logname("fopen", path);
    return real(path, mode);
}
