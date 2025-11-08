#include "log.h"

#include <stdarg.h> // for variadic arguments

#include "util.h"
#include <stdio.h>
#include <stdlib.h>

// global file pointer for the log file
static FILE *g_log = NULL;

int log_init(const char *path) {
    g_log = fopen(path, "a");
    return g_log ? 0 : -1;
}

void log_close(void) {
    if (g_log) {
        fclose(g_log);
        g_log = NULL;
    }
}

/*
Example 1:

void greet(const char *name) {
    printf("Hello, %s!\n", name);
}
- you must pass exactly one argument: the name
- you can t call greet("Alice", "Bob") - it would be an error

Example 2:

#include <stdarg.h>
#include <stdio.h>

void greet_many(int count, ...) {
    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; i++) {
        const char *name = va_arg(args, const char*);
        printf("Hello, %s!\n", name);
    }

    va_end(args);
}
- now you can call: greet_many(3, "Alice", "Bob", "Charlie");
- count tells how many names you re passing
- va_list lets you loop through them one by one
*/

static void vlog_emit(const char *lvl, const char *fmt, va_list ap_in) {
    char ts[32];
    format_utc_now(ts, sizeof ts);   // make sure this NUL-terminates

    // copy BEFORE any vfprintf
    va_list ap1; va_copy(ap1, ap_in);
    va_list ap2; va_copy(ap2, ap_in);

    // stderr (unbuffered-ish)
    fprintf(stderr, "%s %s ", ts, lvl);   // add a space after level
    vfprintf(stderr, fmt, ap1);
    fputc('\n', stderr);
    va_end(ap1);

    if (g_log) {
        fprintf(g_log, "%s %s ", ts, lvl);
        vfprintf(g_log, fmt, ap2);
        fputc('\n', g_log);
        fflush(g_log);
    }
    va_end(ap2);
}

void log_info(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vlog_emit("INFO", fmt, ap);
    va_end(ap);
}

void log_warn(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vlog_emit("WARN", fmt, ap);
    va_end(ap);
}

void log_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vlog_emit("ERROR", fmt, ap);
    va_end(ap);
}