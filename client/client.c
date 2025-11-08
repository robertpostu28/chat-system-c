#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <windows.h>

#include "ui.h"
#include "cmd.h"

#pragma comment(lib, "ws2_32.lib")

static SOCKET g_sock = INVALID_SOCKET;
static HANDLE g_stdin_thread = NULL;

/* stdin reader → cmd_handle_line */
static unsigned __stdcall stdin_thread(void *arg)
{
    (void)arg;
    char line[1024];
    while (fgets(line, sizeof(line), stdin)) {
        int L = (int)strlen(line);
        while (L && (line[L-1]=='\r' || line[L-1]=='\n')) line[--L]=0;
        if (cmd_handle_line(line, g_sock) < 0) break;
    }
    _endthreadex(0);
    return 0;
}

static BOOL WINAPI on_ctrl_c(DWORD type)
{
    (void)type;
    const char *bye = "/quit\n";
    send(g_sock, bye, (int)strlen(bye), 0);
    return TRUE; // prevent default hard-kill, let main exit gracefully
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        return 1;
    }

    printf("Simple Chat Client\n");

    ui_init();
    ui_set_status("anonymous", "general");

    WSADATA wsa; if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) { ui_err("WSAStartup failed"); return 1; }

    const char *host = argv[1];
    unsigned short port = (unsigned short)atoi(argv[2]);

    struct addrinfo hints = {0}, *res = NULL;
    hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, NULL, &hints, &res) != 0) { ui_err("getaddrinfo failed"); WSACleanup(); return 1; }

    struct sockaddr_in serv; memcpy(&serv, res->ai_addr, sizeof(serv)); serv.sin_port = htons(port); freeaddrinfo(res);

    g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_sock == INVALID_SOCKET) { ui_err("socket error: %d", WSAGetLastError()); WSACleanup(); return 1; }

    if (connect(g_sock, (struct sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR) {
        ui_err("connect failed: %d", WSAGetLastError()); closesocket(g_sock); WSACleanup(); return 1;
    }

    char addrstr[INET_ADDRSTRLEN]; inet_ntop(AF_INET, &serv.sin_addr, addrstr, sizeof(addrstr));
    ui_info("Connected to %s:%u", addrstr, (unsigned)port);

    SetConsoleCtrlHandler(on_ctrl_c, TRUE);

    unsigned tid;
    g_stdin_thread = (HANDLE)_beginthreadex(NULL, 0, stdin_thread, NULL, 0, &tid);
    if (!g_stdin_thread) { ui_err("failed to create stdin thread"); closesocket(g_sock); WSACleanup(); return 1; }

    fd_set readset;
    for (;;) {
        FD_ZERO(&readset); FD_SET(g_sock, &readset);
        int ret = select(0, &readset, NULL, NULL, NULL);
        if (ret == SOCKET_ERROR) break;

        if (FD_ISSET(g_sock, &readset)) {
            char buf[2048];
            int n = recv(g_sock, buf, sizeof(buf), 0);
            if (n <= 0) break;
            ui_render_server_line(buf, n);
        }
    }

    /* cleanup */
    WaitForSingleObject(g_stdin_thread, 1500);
    CloseHandle(g_stdin_thread);

    if (g_sock != INVALID_SOCKET) closesocket(g_sock);
    WSACleanup();
    ui_warn("Disconnected.");
    return 0;
}
