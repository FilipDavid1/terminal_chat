#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <time.h>

#include "chat.h"

#define BACKLOG 16
#define PORT_STR_LEN 16

typedef enum {
    MODE_UNIX = 0,
    MODE_TCP = 1
} ServerMode;

typedef struct Client {
    int fd;
    char username[USERNAME_MAX];
    pthread_t thread;
    time_t last_activity;
    int removed;
    struct Client *next;
} Client;

#endif

