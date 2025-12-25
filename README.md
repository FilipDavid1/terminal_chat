# Multi-Process Terminal Chat Server

## What it is
- Terminal chat with a server process and multiple client processes.
- Uses POSIX threads and IPC (UNIX domain sockets by default; TCP optional).
- Messages are broadcast or private (`@user`) using a fixed `ChatMessage` struct.

## Build
```sh
make
```

## Run (basics)
```sh
# Server (UNIX socket default)
./server [--unix /tmp/pos_chat.sock] [--timeout 300]
# Server (TCP)
./server --tcp 5555 [--timeout 300]

# Clients
./client alice
./client bob --unix /tmp/pos_chat.sock
./client carol --tcp 192.168.1.10 5555
```

Useful client commands: `/help`, `/quit`, `@user msg`, plain text for broadcast.

