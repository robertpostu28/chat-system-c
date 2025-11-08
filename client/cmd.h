#ifndef LAB3_CMD_H
#define LAB3_CMD_H

#include <winsock2.h>

// parse a user-typed line, support a few client-side helpers, then send
// returns 0 on success, -1 on fatal send error
int cmd_handle_line(const char *line, SOCKET sock);

#endif //LAB3_CMD_H