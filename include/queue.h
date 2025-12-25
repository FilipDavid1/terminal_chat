#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

#include "chat.h"

typedef struct MessageNode {
    ChatMessage msg;
    struct MessageNode *next;
} MessageNode;

typedef struct MessageQueue {
    MessageNode *head;
    MessageNode *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int closed;
} MessageQueue;

void mq_init(MessageQueue *queue);
void mq_push(MessageQueue *queue, const ChatMessage *msg);
int mq_pop(MessageQueue *queue, ChatMessage *out); /* returns 0 on success, -1 if closed */
void mq_close(MessageQueue *queue);
void mq_destroy(MessageQueue *queue);

#endif


