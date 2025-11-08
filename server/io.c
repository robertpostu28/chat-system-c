#include "io.h"

#include <errno.h> // for errno and EINTR
#include <unistd.h> // for read() and write()

ssize_t read_n(int fd, void *buffer, size_t  n) {
    size_t got = 0; // total bytes read so far
    char *p = (char *)buffer; // pointer to buffer as byte array

    while (got < n) {
        // attempt to read remaining bytes
        ssize_t r = read(fd, p + got, n - got);

        if (r == 0) return 0; // EOF reached
        if (r < 0) {
            if (errno == EINTR) continue; // interrupted by signal, retry
            return -1; // other error
        }
        got += (size_t)r; // accumulate bytes read
    }

    return (ssize_t)got; // total bytes read
}

ssize_t write_all(int fd, const void *buffer, size_t n) {
    size_t sent = 0; // total bytes written so far
    const char *p = (const char *)buffer; // pointer to buffer as byte array

    while (sent < n) {
        // attempt to write remaining bytes
        ssize_t w = write(fd, p + sent, n - sent);

        if (w < 0) {
            if (errno == EINTR) continue; // interrupted by signal, retry
            return -1; // other error
        }
        sent += (size_t)w; // accumulate bytes written
    }

    return (ssize_t)sent; // return total bytes written
}