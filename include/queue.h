#ifndef QUEUE_H
#define QUEUE_H

#include "chat.h"

/* Opaque pointer - internal structure hidden from users */
typedef struct MessageQueue MessageQueue;

/* Create and destroy queue */
MessageQueue *mq_create(void);
void mq_destroy(MessageQueue *queue);

/* Queue operations */
void mq_push(MessageQueue *queue, const ChatMessage *msg);
int mq_pop(MessageQueue *queue, ChatMessage *out); /* returns 0 on success, -1 if closed */
void mq_close(MessageQueue *queue);

#endif


