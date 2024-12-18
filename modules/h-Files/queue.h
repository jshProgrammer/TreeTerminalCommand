#ifndef QUEUE_H
#define QUEUE_H
#define MAX_PATH
#include <pthread.h>

typedef struct QueueNode {
    int level;
    struct QueueNode *next;
    char path[MAX_PATH];
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int is_finished;
} Queue;

void queue_init(Queue *q);
void enqueue(Queue *q, const char *path, int level);
int dequeue(Queue *q, char *path, int *level);
void queue_destroy(Queue *q);


#endif // QUEUE_H
