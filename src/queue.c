#include <stdlib.h>
#include <string.h>

#include "queue.h"

void mq_init(MessageQueue *queue) {
    queue->head = NULL;
    queue->tail = NULL;
    queue->closed = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void mq_push(MessageQueue *queue, const ChatMessage *msg) {
    MessageNode *node = malloc(sizeof(MessageNode));
    if (!node) {
        return;
    }
    memcpy(&node->msg, msg, sizeof(ChatMessage));
    node->next = NULL;

    pthread_mutex_lock(&queue->mutex);
    if (queue->closed) {
        pthread_mutex_unlock(&queue->mutex);
        free(node);
        return;
    }

    if (queue->tail) {
        queue->tail->next = node;
        queue->tail = node;
    } else {
        queue->head = queue->tail = node;
    }
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

int mq_pop(MessageQueue *queue, ChatMessage *out) {
    pthread_mutex_lock(&queue->mutex);
    while (!queue->head && !queue->closed) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    if (!queue->head) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    MessageNode *node = queue->head;
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    pthread_mutex_unlock(&queue->mutex);

    memcpy(out, &node->msg, sizeof(ChatMessage));
    free(node);
    return 0;
}

void mq_close(MessageQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    queue->closed = 1;
    pthread_cond_broadcast(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

void mq_destroy(MessageQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    MessageNode *node = queue->head;
    while (node) {
        MessageNode *next = node->next;
        free(node);
        node = next;
    }
    queue->head = queue->tail = NULL;
    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
}


