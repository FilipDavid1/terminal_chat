#define _GNU_SOURCE
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "chat.h"
#include "queue.h"
#include "server.h"

static int server_fd = -1;
static volatile sig_atomic_t running = 1;
static pthread_t accept_thread_id;
static pthread_t dispatcher_thread_id;
static pthread_t logger_thread_id;
static pthread_t watchdog_thread_id;

static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static Client *clients = NULL;

static MessageQueue dispatch_queue;
static MessageQueue log_queue;

static time_t inactivity_timeout_sec = 300; /* default 5 minutes */
static ServerMode server_mode = MODE_UNIX;
static char server_unix_path[sizeof(((struct sockaddr_un *)0)->sun_path)] = SOCKET_PATH;
static char server_tcp_port[PORT_STR_LEN] = DEFAULT_TCP_PORT;

static int setup_unix_socket(const char *path) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
    unlink(path);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return -1;
    }

    if (listen(fd, BACKLOG) < 0) {
        perror("listen");
        close(fd);
        return -1;
    }

    return fd;
}

static int setup_tcp_socket(const char *port) {
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int gai = getaddrinfo(NULL, port, &hints, &res);
    if (gai != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
        return -1;
    }

    int fd = -1;
    for (struct addrinfo *p = res; p; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) {
            continue;
        }
        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (bind(fd, p->ai_addr, p->ai_addrlen) == 0) {
            if (listen(fd, BACKLOG) == 0) {
                break;
            }
        }
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);

    if (fd < 0) {
        perror("bind/listen");
    }
    return fd;
}
