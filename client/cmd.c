#include "cmd.h"

#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <windows.h>

static int send_all(SOCKET s, const char *buf, int len) {
    int sent = 0;
    while (sent < len) {
        int r = send(s, buf + sent, len - sent, 0);
        if (r == SOCKET_ERROR) return SOCKET_ERROR;
        sent += r;
    }
    return sent;
}

int cmd_handle_line(const char *line, SOCKET sock) {
    if (!line || !*line) return 0;

    // client side
    if (_stricmp(line, "/clear") == 0) {
        system("clear");
        ui_set_status(NULL, NULL);
        return 0;
    }

    if (_stricmp(line, "/help") == 0) {
        ui_info("Client helpers: /clear, /help. Server: /login, /who, /join, /msg, /ping, /quit");
        return 0;
    }

    // send original line + newline to server
    char out[1024];
    int n = _snprintf_s(out, sizeof(out), _TRUNCATE, "%s\n", line);
    if (n <= 0) return 0;

    if (send_all(sock, out, n) == SOCKET_ERROR) {
        ui_err("send error: %d", WSAGetLastError());
        return -1;
    }
    return 0;
}