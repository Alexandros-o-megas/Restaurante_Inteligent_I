#ifndef BUFFER_H
#define BUFFER_H

#include <semaphore.h>
#include <pthread.h>

#define BUF_SIZE 5

typedef struct {
    int dados[BUF_SIZE];
    int in;
    int out;

    sem_t empty;
    sem_t full;
    pthread_mutex_t mutex;
} Buffer;

void buffer_init(Buffer *b);

void buffer_put(Buffer *b, int item);

int buffer_get(Buffer *b);

int buffer_count(Buffer *b);

void buffer_destroy(Buffer *b);

#endif /* BUFFER_H */