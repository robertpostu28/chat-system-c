#ifndef LAB3_IO_H
#define LAB3_IO_H

#include <stdio.h>

// reads exactly "n" bytes from file descriptor "fd" into "buffer"
// returns number of bytes read, or -1 on error, 0 on EOF (End Of File)
ssize_t read_n(int fd, void *buffer, size_t n);

// writes exactly "n" bytes from "buffer" to file descriptor "fd"
// returns number of bytes written, or -1 on error
ssize_t write_all(int fd, const void *buffer, size_t n);

#endif //LAB3_IO_H