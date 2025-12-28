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
