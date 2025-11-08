#ifndef LAB3_ACCEPT_H
#define LAB3_ACCEPT_H

// accepts a new incoming client connection on the given listener socket
// returns the new client's file descriptor, or -1 on error
int accept_new_client(int listener);

#endif //LAB3_ACCEPT_H