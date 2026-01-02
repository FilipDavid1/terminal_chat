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
#include "client.h"

static volatile sig_atomic_t running = 1;
static int server_fd = -1;
static char username[USERNAME_MAX];

static ClientMode client_mode = MODE_UNIX;
static char server_unix_path[sizeof(((struct sockaddr_un *)0)->sun_path)] = SOCKET_PATH;
static char server_tcp_host[256] = "127.0.0.1";
static char server_tcp_port[16] = DEFAULT_TCP_PORT;

static void handle_sigint(int sig) {
    (void)sig;
    running = 0;
    if (server_fd >= 0) {
        shutdown(server_fd, SHUT_RDWR);
    }
}

static int connect_unix_socket(const char *path) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }

    return fd;
}

static int connect_tcp_socket(const char *host, const char *port) {
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int gai = getaddrinfo(host, port, &hints, &res);
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
        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);

    if (fd < 0) {
        perror("connect");
    }
    return fd;
}

static void strip_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

static void format_time(time_t ts, char *buf, size_t len) {
    struct tm tm_info;
    localtime_r(&ts, &tm_info);
    strftime(buf, len, "%H:%M:%S", &tm_info);
}

static void *receiver_thread(void *arg) {
    (void)arg;
    ChatMessage msg;
    char timebuf[32];
    while (running) {
        if (recv_all(server_fd, &msg, sizeof(ChatMessage)) < 0) {
            fprintf(stderr, "Connection lost.\n");
            running = 0;
            break;
        }
        msg.sender[USERNAME_MAX - 1] = '\0';
        msg.target[USERNAME_MAX - 1] = '\0';
        msg.text[TEXT_MAX - 1] = '\0';
        format_time(msg.timestamp, timebuf, sizeof(timebuf));
        if (msg.target[0] && strncmp(msg.target, username, USERNAME_MAX) == 0) {
            printf("[%s] (private) <%s> %s\n", timebuf, msg.sender, msg.text);
        } else if (msg.target[0]) {
            printf("[%s] <%s -> %s> %s\n", timebuf, msg.sender, msg.target, msg.text);
        } else {
            printf("[%s] <%s> %s\n", timebuf, msg.sender, msg.text);
        }
        fflush(stdout);
    }
    return NULL;
}
