#include <stdlib.h>
#include <string.h>
#include "h-Files/queue.h"
#include "h-Files/globals.h"
#include <stdio.h>


void queue_init(Queue *q) {
    q->front = q->rear = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void enqueue(Queue *q, const char *path, int level) {
    QueueNode *node = malloc(sizeof(QueueNode));

    if (!node) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    strncpy(node->path, path, MAX_PATH-1);
    node->path[MAX_PATH - 1] = '\0';  // Ensure null-termination
    node->level = level;
    node->next = NULL;

    pthread_mutex_lock(&q->lock);
    if (q->rear) {
        q->rear->next = node;
    } else {
        q->front = node;
    }
    q->rear = node;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->lock);
}

int dequeue(Queue *q, char *path, int *level) {
    pthread_mutex_lock(&q->lock);

    while (q->front == NULL && !q->is_finished) {
        pthread_cond_wait(&q->cond, &q->lock);
    }

    // Exit if queue is empty and finished
    if (q->front == NULL && q->is_finished) {
        pthread_mutex_unlock(&q->lock);
        return 0;
    }

    QueueNode *node = q->front;
    strncpy(path, node->path, max_path);
    *level = node->level;

    q->front = node->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(node);
    pthread_mutex_unlock(&q->lock);
    return 1; // Erfolgreich ein Element entnommen
}

void queue_destroy(Queue *q) {
    pthread_mutex_lock(&q->lock);
    QueueNode *current = q->front;
    while (current) {
        QueueNode *next = current->next;
        free(current);
        current = next;
    }
    // Explicitly reset queue state
    q->front = q->rear = NULL;
    pthread_mutex_unlock(&q->lock);

    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->cond);
}