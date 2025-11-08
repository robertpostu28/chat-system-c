#include "registry.h"

#include <string.h> // for memset
#include <sys/select.h> // for FD_SETSIZE
#include <ctype.h>

#define MAX_CLIENTS FD_SETSIZE // FD_SETSIZE defines the maximum number of file descriptors that can be tracked, often 1024
static Client clients[MAX_CLIENTS]; // array of client slots, indexed by file descriptor

void registry_init(void) {
    memset(clients, 0, sizeof(clients)); // set all bytes to zero
}

void registry_add(int fd) {
    if (fd >= 0 && fd < MAX_CLIENTS) {
        clients[fd].fd = fd; // mark this slot active
        clients[fd].name[0] = 0;
        strcpy(clients[fd].room, "general"); // default room
        clients[fd].last_seen = time(NULL);
    }
}

void registry_remove(int fd) {
    if (fd >= 0 && fd < MAX_CLIENTS) {
        clients[fd].fd = 0; // mark this slot as inactive
    }
}

// checks if a client with the given file descriptor exists
bool registry_exists(int fd) {
    return (fd >= 0 && fd < MAX_CLIENTS && clients[fd].fd == fd);
}

// returns a pointer to the Client struct for a given fd
Client* registry_by_fd(int fd) {
    return registry_exists(fd) ? &clients[fd] : NULL;
}

// case-insensitive name comparison
// "robert" and "Robert" are treated as equal
static int name_eq(const char *a, const char *b) {
    for (; *a && *b; a++, b++) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);

        if (ca != cb) {
            return 0; // missmatch
        }
    }

    return *a == 0 && *b == 0; // both strings ended together
}

Client* registry_by_name(const char *name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd && clients[i].name[0] && name_eq(name, clients[i].name)) {
            return &clients[i]; // found match
        }
    }
    return NULL; // not found
}

bool registry_set_name(int fd, const char *name) {
    if (!registry_exists(fd) || !name || !*name) {
        return false; // invalid fd or empty name
    }

    if (strlen(name) > MAX_NAME) {
        return false; // too long
    }

    if (registry_by_name(name)) {
        return false; // name already taken
    }

    // basic sanity: letters, digits, underscore
    for (const char *p = name; *p; p++) {
        if (!(isalnum((unsigned char)*p) || *p == '-' || *p == '_')) {
            return false; // invalid character
        }
    }

    strcpy(clients[fd].name, name); // assign name
    return true;
}