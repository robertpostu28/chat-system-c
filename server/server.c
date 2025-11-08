#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>    // strcasecmp / strncasecmp
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#include "accept.h"
#include "registry.h"
#include "router.h"
#include "io.h"
#include "util.h"
#include "log.h"

#define TIMEOUT_SECS 60
#define SWEEP_PERIOD 5


// globals shared with other modules
fd_set g_master;         // all active FDs (clients + listener)
fd_set read_fds;         // temp set for select()
int    g_fdmax;          // max FD
int    listener;         // listening socket
char   buf[256], tmpbuf[256];
int    client_count = 0;

static time_t last_sweep = 0;

// case-insensitive prefix check
static int has_prefix_ci(const char *s, const char *pref) {
    size_t n = strlen(pref);
    return strncasecmp(s, pref, n) == 0;
}

static const char* display_name(const Client *c) {
    return (c && c->name[0]) ? c->name : "anonymous";
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // avoid SIGPIPE when a client drops between select() and write()
    signal(SIGPIPE, SIG_IGN);

    int port = atoi(argv[1]);
    int yes  = 1;

    FD_ZERO(&g_master);
    FD_ZERO(&read_fds);
    registry_init();

    // listener
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1) { perror("socket"); exit(1); }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt SO_REUSEADDR");
        exit(1);
    }

    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family      = AF_INET;
    myaddr.sin_port        = htons(port);
    myaddr.sin_addr.s_addr = INADDR_ANY;

    if (log_init("server.log") != 0) {
        fprintf(stderr, "failed to open server.log\n");
        // we continue but without the file logs
    }
    log_info("boot: listening planned on port %d", port);

    if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
        perror("bind");
        exit(1);
    }
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Server listening on 0.0.0.0:%d\n", ntohs(myaddr.sin_port));
    log_info("listening on 0.0.0.0:%d", ntohs(myaddr.sin_port));

    FD_SET(listener, &g_master);
    g_fdmax = listener;

    // main loop
    for (;;) {
        read_fds = g_master;
        int sel;
        do { sel = select(g_fdmax + 1, &read_fds, NULL, NULL, NULL); }
        while (sel == -1 && errno == EINTR);
        if (sel == -1) { perror("select"); exit(1); }

        for (int i = 0; i <= g_fdmax; i++) {
            if (!FD_ISSET(i, &read_fds)) continue;

            if (i == listener) {
                // new connection
                int new_fd = accept_new_client(listener);
                if (new_fd >= 0) {
                    FD_SET(new_fd, &g_master);
                    if (new_fd > g_fdmax) g_fdmax = new_fd;
                    client_count++;

                    log_info("accept fd=%d", new_fd);
                    snprintf(buf, sizeof(buf),
                             "Hi - you are client [%d]\nThere are %d clients connected\n",
                             new_fd, client_count);
                    write_all(new_fd, buf, strlen(buf));
                }
            } else {
                // client data
                ssize_t nbytes = recv(i, buf, sizeof(buf) - 1, 0);
                if (nbytes <= 0) {
                    if (nbytes == 0) {
                        printf("Client %d disconnected\n", i);
                        log_info("disconnected fd=%d\n", i);
                    } else {
                        perror("recv");
                        log_warn("recv error fd=%d errno=%d (%s)", i, errno, strerror(errno));
                    }
                    if (client_count > 0) client_count--;
                    close(i);
                    FD_CLR(i, &g_master);
                    registry_remove(i);
                } else {
                    buf[nbytes] = 0;
                    str_trim(buf);  // trims spaces + CR/LF (from util.h)

                    Client *me = registry_by_fd(i);
                    if (me) me->last_seen = time(NULL);

                    if (buf[0] == '/') {
                        // commands
                        if (has_prefix_ci(buf, "/login")) {
                            const char *name = buf + 6;   // skip "/login"
                            while (*name == ' ') name++;  // skip spaces
                            if (*name == '\0') {
                                const char *err = "[ERROR] usage: /login <name>\n";
                                write_all(i, err, strlen(err));
                            } else if (registry_set_name(i, name)) {
                                snprintf(tmpbuf, sizeof(tmpbuf), "[SYSTEM] you are now '%s'\n", name);
                                write_all(i, tmpbuf, strlen(tmpbuf));

                                log_info("login ok fd=%d name=%s", i, name);
                                snprintf(tmpbuf, sizeof(tmpbuf), "[SYSTEM] %s joined\n", name);
                                broadcast_all(i, tmpbuf, strlen(tmpbuf));
                            } else {
                                const char *err = "[ERROR] invalid or duplicate username\n";
                                write_all(i, err, strlen(err));
                                log_warn("login failed  fd=%d reason=duplicate_or_invalid name_try=%s", i, name);
                            }
                        } else if (has_prefix_ci(buf, "/who")) {
                            const char *arg = buf + 4; while (*arg==' ') arg++;
                            const char *room = (*arg) ? arg : (me && me->room[0] ? me->room : "general");

                            log_info("who fd=%d room=%s", i, room);
                            char list[1024]; size_t off=0; int count=0;
                            off += snprintf(list+off, sizeof list-off, "[SYSTEM] users in #%s:\n", room);

                            for (int fd=0; fd<=g_fdmax; fd++){
                                Client *c = registry_by_fd(fd);
                                if (c && c->fd && strcmp(c->room, room)==0) {
                                    off += snprintf(list+off, sizeof list-off, " - %s\n", display_name(c));
                                    count++;
                                }
                            }
                            if (count==0) off += snprintf(list+off, sizeof list-off, " (none)\n");
                            write_all(i, list, (int)off);
                        }
                        else if (has_prefix_ci(buf, "/quit")) {
                            snprintf(buf, sizeof(buf), "Goodbye [%d]\n", i);
                            write_all(i, buf, strlen(buf));

                            log_info("quit fd=%d", i);
                            snprintf(tmpbuf, sizeof(tmpbuf), "<[%d]> disconnected\n", i);
                            broadcast_all(i, tmpbuf, strlen(tmpbuf));
                            printf("%s", tmpbuf);

                            if (client_count > 0) client_count--;
                            close(i);
                            FD_CLR(i, &g_master);
                            registry_remove(i);
                        } else if (has_prefix_ci(buf, "/join")) {
                            Client *me = registry_by_fd(i);
                            const char *arg = buf + 5; while (*arg==' ') arg++;

                            if (!me || !me->name[0]) {
                                write_all(i,"[ERROR] /login first\n", strlen("[ERROR] /login first\n"));
                                continue;
                            } else if (*arg=='\0' || strlen(arg) >= MAX_ROOM) {
                                write_all(i, "[ERROR] usage: /join <room>\n", strlen("[ERROR] /join <room>\n"));
                            } else {
                                const char *old = me->room[0] ? me->room : "general";
                                if (strcmp(old, arg)==0) {
                                    char m[128];
                                    int n = snprintf(m, sizeof m, "[SYSTEM] already in #%s\n", old);
                                    write_all(i, m, n);
                                } else {
                                    // notify old room
                                    char m[256];
                                    int n = snprintf(m, sizeof m, "[SYSTEM] %s left #%s\n", display_name(me), old);
                                    broadcast_room(old, i, m, n);

                                    // set new room
                                    strncpy(me->room, arg, MAX_ROOM-1);
                                    me->room[MAX_ROOM-1] = 0;

                                    // notify new room (others)
                                    n = snprintf(m, sizeof m, "[SYSTEM] %s joined #%s\n", display_name(me), me->room);
                                    broadcast_room(me->room, i, m, n);

                                    // confirm to self
                                    n = snprintf(m, sizeof m, "[SYSTEM] you are now in #%s\n", me->room);
                                    write_all(i, m, n);

                                    log_info("join fd=%d name=%s %s -> %s", i, display_name(me), old, me->room);
                                }
                            }
                        } else if (has_prefix_ci(buf, "/ping")) {
                            if (me) me->last_seen = time(NULL);
                            // optional: reply so NATs stay open
                            write_all(i, "[PONG]\n", 7);
                        } else if (has_prefix_ci(buf, "/msg")) {
                            const char* p = buf + 4;
                            while (*p == ' ') p++;
                            const char *sp = strchr(p, ' ');
                            if (!sp) {
                                write_all(i, "[ERROR] usage: /msg <user> <message>\n", strlen("[ERROR] usage: /msg <user> <message>"));
                            } else {
                                char target[64];
                                size_t n = (size_t)(sp - p);
                                if (n >= sizeof(target)) n = sizeof(target) - 1;
                                memcpy(target, p, n);
                                target[n] = 0;
                                const char *msg = sp + 1;

                                Client *source = registry_by_fd(i);
                                Client *destination = registry_by_name(target);

                                if (!source || !source->name[0]) {
                                    write_all(i, "[ERROR] /login first\n", strlen("[ERROR] /login first\n"));
                                } else if (!destination) {
                                    char e[128];
                                    int m = snprintf(e, sizeof(e), "[ERROR] user '%s' not found\n", target);
                                    write_all(i, e, m);
                                } else {
                                    char out[512];
                                    int m = snprintf(out, sizeof out, "[PM from %s] %s\n", display_name(source), msg);
                                    write_all(destination->fd, out, m);   // to target

                                    m = snprintf(out, sizeof out, "[PM to %s] %s\n", display_name(destination), msg);
                                    write_all(i, out, m);                  // confirm to sender
                                    log_info("pm %s -> %s: %s", display_name(source), display_name(destination), msg);

                                }
                            }
                        } else {
                            const char *help =
                            "[HELP] commands: /login <name>, /who [room], /join <room>, /msg <user> <text>, /ping, /quit\n";

                            write_all(i, help, strlen(help));
                        }
                    } else {
                        // plain message -> broadcast, show username if set
                        Client *c = registry_by_fd(i);
                        const char *who  = display_name(c);
                        const char *room = (c && c->room[0]) ? c->room : "general";

                        int n = snprintf(tmpbuf, sizeof tmpbuf, "<%s> %s\n", who, buf);
                        printf("[#%s] %s", room, tmpbuf);
                        broadcast_room(room, i, tmpbuf, n);

                        log_info("msg room=%s from=%s: %s", room, who, buf);
                    }
                }
            }
        }

        time_t now = time(NULL);
        if (now - last_sweep >= SWEEP_PERIOD) {
            last_sweep = now;
            for (int fd = 0; fd <= g_fdmax; fd++) {
                if (!FD_ISSET(fd, &g_master) || fd == listener) continue;
                Client *c = registry_by_fd(fd);
                if (c && c->fd && (now - c->last_seen) > TIMEOUT_SECS) {
                    int n = snprintf(tmpbuf, sizeof tmpbuf,
                                     "[SYSTEM] %s timed out\n", display_name(c));
                    const char *room = (c->room[0] ? c->room : "general");
                    broadcast_room(room, fd, tmpbuf, n);

                    if (client_count > 0) client_count--;
                    close(fd);
                    FD_CLR(fd, &g_master);
                    registry_remove(fd);

                    log_warn("timeout fd=%d name=%s", fd, display_name(c));
                }
            }
        }

    }

    log_close();
}
