#ifndef LAB3_UI_H
#define LAB3_UI_H

#include <windows.h>

// initialize console: enable ANSI colors on win10+, set stdin to binary mode
void ui_init();


// prompt status (ex: current user/room)
void ui_set_status(const char *user, const char *room);

// print helpers (auto add newline if missing)
void ui_info(const char *fmt, ...);
void ui_warn(const char *fmt, ...);
void ui_err(const char *fmt, ...);

/*
 * Render a server line with basic highlighting:
 * [SYSTEM] -> yellow
 * [PM ...] -> magenta
 * <name>   -> green name, dim message
 * other    -> plain
 */
void ui_render_server_line(const char *buffer, int n);

#endif //LAB3_UI_H