CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -pedantic -pthread -Iinclude
LDFLAGS := -pthread

SERVER_BIN := server
CLIENT_BIN := client

SERVER_SRCS := src/server.c src/queue.c src/ipc.c
CLIENT_SRCS := src/client.c src/ipc.c

.PHONY: all server client clean

all: server client

server: $(SERVER_SRCS) include/chat.h include/queue.h
	$(CC) $(CFLAGS) -o $(SERVER_BIN) $(SERVER_SRCS) $(LDFLAGS)

client: $(CLIENT_SRCS) include/chat.h
	$(CC) $(CFLAGS) -o $(CLIENT_BIN) $(CLIENT_SRCS) $(LDFLAGS)

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN) chat.log


