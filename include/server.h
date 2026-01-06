#ifndef SERVER_H
#define SERVER_H

#include "chat.h"

#define BACKLOG 16
#define PORT_STR_LEN 16

typedef enum {
    MODE_UNIX = 0,
    MODE_TCP = 1
} ServerMode;

#endif

