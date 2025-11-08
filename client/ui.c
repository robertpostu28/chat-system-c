#include "ui.h"

#include <stdio.h>
#include <stdarg.h>

static char g_user[64] = "anonymous";
static char g_room[64] = "general";

static void add_nl_if_needed(const char *prefix, const char *fmt, va_list ap, const char *color) {
    // print prefix + formatted message + newline, with color if available
    if (color) fputs(color, stdout);
    if (prefix) fputs(prefix, stdout);

    vfprintf(stdout, fmt, ap);

    // ensure newline
    int needs_nl = 1;
    size_t len = strlen(fmt);
    if (len && fmt[len - 1] == '\n') needs_nl = 0;
    if (needs_nl) fputc('\n', stdout);
    if (color) fputs("\x1b[0m", stdout); // reset
    fflush(stdout);
}

void ui_init(void) {
    // turns on ANSI escapes on Windows 10+
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
}

void ui_set_status(const char *user, const char *room) {
    if (user && *user) {
        strncpy(g_user, user, sizeof(g_user) - 1);
        g_user[sizeof(g_user) - 1] = '\0';
    }
    if (room && *room) {
        strncpy(g_room, room, sizeof(g_room) - 1);
        g_room[sizeof(g_room) - 1] = '\0';
    }

    // simple status line
    printf("\x1b[2K\r\x1b[90m[%s@%s]\x1b[0m ", g_user, g_room);
    fflush(stdout);
}

void ui_info(const char *fmt, ...)
{
    va_list ap; va_start(ap, fmt);
    add_nl_if_needed(NULL, fmt, ap, "\x1b[36m"); // cyan
    va_end(ap);
    ui_set_status(NULL, NULL);
}

void ui_warn(const char *fmt, ...)
{
    va_list ap; va_start(ap, fmt);
    add_nl_if_needed(NULL, fmt, ap, "\x1b[33m"); // yellow
    va_end(ap);
    ui_set_status(NULL, NULL);
}

void ui_err(const char *fmt, ...)
{
    va_list ap; va_start(ap, fmt);
    add_nl_if_needed(NULL, fmt, ap, "\x1b[31m"); // red
    va_end(ap);
    ui_set_status(NULL, NULL);
}

static int starts_with(const char *s, const char *p) {
    return _strnicmp(s, p, (unsigned) strlen(p)) == 0;
}

void ui_render_server_line(const char *buf, int n)
{
    /* Copy into a zero-terminated scratch */
    char line[2048];
    if (n <= 0) return;
    if (n >= (int)sizeof(line)) n = (int)sizeof(line)-1;
    memcpy(line, buf, n);
    line[n] = 0;

    /* Update local prompt hints on certain server messages */
    if (starts_with(line, "[SYSTEM] you are now '")) {
        const char *q = strchr(line, '\'');
        if (q) {
            const char *q2 = strchr(q+1, '\'');
            if (q2 && q2 > q+1) {
                char name[64]; size_t L = (size_t)(q2-(q+1));
                if (L >= sizeof(name)) L = sizeof(name)-1;
                memcpy(name, q+1, L); name[L]=0;
                ui_set_status(name, NULL);
            }
        }
    } else if (starts_with(line, "[SYSTEM] you are now in #")) {
        const char *h = strchr(line, '#');
        if (h) {
            char room[64]; size_t L = strcspn(h+1, "\r\n");
            if (L >= sizeof(room)) L = sizeof(room)-1;
            memcpy(room, h+1, L); room[L]=0;
            ui_set_status(NULL, room);
        }
    }

    /* Highlight categories */
    if (starts_with(line, "[SYSTEM]")) {
        fputs("\x1b[33m", stdout);      // yellow
        fputs(line, stdout);
        fputs("\x1b[0m\n", stdout);
    } else if (starts_with(line, "[PM ")) {
        fputs("\x1b[35m", stdout);      // magenta
        fputs(line, stdout);
        fputs("\x1b[0m\n", stdout);
    } else if (line[0] == '<') {
        /* Colorize the <name> part */
        const char *gt = strchr(line, '>');
        if (gt) {
            fputs("\x1b[32m", stdout);              // green name
            fwrite(line, 1, (size_t)(gt - line + 1), stdout);
            fputs("\x1b[90m", stdout);              // dim message
            fputs(line + (gt - line + 1), stdout);
            fputs("\x1b[0m\n", stdout);
        } else {
            puts(line);
        }
    } else {
        puts(line);
    }
    fflush(stdout);
    ui_set_status(NULL, NULL);
}
