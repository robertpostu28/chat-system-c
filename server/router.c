#include "router.h"

#include <string.h>

#include "io.h" // for write_all()
#include "registry.h" // for client tracking
#include <sys/select.h> // for FD_ISSET and fd_set
#include <unistd.h> // for file descriptor operations

extern fd_set g_master; // set of all active file descriptors (from server.c)
extern int g_fdmax; // highest-numbered file descriptor in use

extern int listener;

int broadcast_all(int except_fd, const char *buffer, int len) {
    for (int i = 0; i <= g_fdmax; i++) {
        if (FD_ISSET(i, &g_master) && i != except_fd && i != listener) {
            if (write_all(i, buffer, (size_t)len) < 0) {
                // ignore or log later
            }
        }
    }
    return 0;
}

int broadcast_room(const char *room, int sender_fd, const char *message, int message_length) {
    for (int fd = 0; fd <= g_fdmax; fd++) {
        if (FD_ISSET(fd, &g_master) && fd != sender_fd && fd != listener) {
            Client *c = registry_by_fd(fd);
            if (c && strcmp(c->room, room) == 0) {
                if (write_all(fd, message, (size_t)message_length) < 0) {
                    //TODO: optinal cleanup/log
                }
            }
        }
    }

    return 0;
}