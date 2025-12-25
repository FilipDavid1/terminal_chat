#ifndef CHAT_H
#define CHAT_H

#include <stddef.h>
#include <time.h>

#define USERNAME_MAX 32
#define TEXT_MAX 256
#define SOCKET_PATH "/tmp/pos_chat.sock"
#define DEFAULT_TCP_PORT "5555"

typedef struct ChatMessage {
    char sender[USERNAME_MAX];
    char target[USERNAME_MAX]; /* empty means broadcast */
    char text[TEXT_MAX];
    time_t timestamp;
} ChatMessage;

int send_all(int fd, const void *buf, size_t len);
int recv_all(int fd, void *buf, size_t len);

#endif


