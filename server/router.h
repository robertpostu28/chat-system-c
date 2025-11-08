#ifndef LAB3_ROUTER_H
#define LAB3_ROUTER_H

// sends a message to all connected clients except the one with "except_fd"
// returns 0 on success (errors are ignored for now)
int broadcast_all(int except_fd, const char *buffer, int len);

int broadcast_room(const char *room, int sender_fd, const char *message, int message_length);

#endif //LAB3_ROUTER_H