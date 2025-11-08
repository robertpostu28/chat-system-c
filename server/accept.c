#include "accept.h"

#include "log.h" // for logging connection info or errors
#include "registry.h" // to register the new client
#include <sys/socket.h> // for accept(), sockaddr
#include <netinet/in.h> // for sockaddr_in
#include <arpa/inet.h> // for inet_ntoa()

int accept_new_client(int listener) {
    struct sockaddr_in addr; // structure to hold client address info
    socklen_t addrLen = sizeof(addr); // length of the address structure

    // accept a new connection on the listener socket
    int new_fd = accept(listener, (struct sockaddr *) &addr, &addrLen);

    if (new_fd < 0) {
        log_error("accept failed"); // log the error
        return -1; // return failure
    }

    // lof the client's IP address and port
    log_info("connect fd=%d from %s:%d", new_fd, inet_ntoa(addr.sin_addr), (int)ntohs(addr.sin_port));

    // register the new client in the registry
    registry_add(new_fd);

    return new_fd; // return the new client's file descriptor
}