#ifndef LAB3_REGISTRY_H
#define LAB3_REGISTRY_H

#include <stdbool.h> // for using "bool" type
#include <time.h>

#define MAX_NAME 32
#define MAX_ROOM 32

// structure representing a connected client
typedef struct {
    int fd; // file descriptor identifying the client connection
    char name[MAX_NAME]; // empty = not logged in
    char room[MAX_ROOM]; // current room
    time_t last_seen; // for heartbeat
} Client;

// initializes the client registry (clears all entries)
void registry_init(void);

// adds a client to the registry using its file descriptor
void registry_add(int fd);

// removes a client from the registry by file descriptor
void registry_remove(int fd);

// checks if a client with the given file descriptor exists in the registry
bool registry_exists(int fd);

// returns a pointer to the Client struct for a given fd
Client* registry_by_fd(int fd);

// returns a pointer to the Client struct for a given username
Client* registry_by_name(const char *name); // null is not found

// sets the username for a client with the given fd
// returns false if name is invalid or already taken
bool registry_set_name(int fd, const char *name);

#endif //LAB3_REGISTRY_H