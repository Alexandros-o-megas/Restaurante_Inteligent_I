#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>     

void buffer_init(Buffer *b) {
    b->in = 0;
    b->out = 0;

    if(sem_init(&b->empty, 0, BUF_SIZE) !=0) {
        perror("sem_init empty");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&b->full, 0, 0) !=0) {
        perror("sem_init full");
        exit(EXIT_FAILURE);
    }
    if(pthread_mutex_init(&b->mutex, NULL) !=0) {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }
}

void buffer_put(Buffer *b, int item) {
    sem_wait(&b->empty);

    pthread_mutex_lock(&b->mutex);
    b->dados[b->in] = item;
    b->in = (b->in + 1) % BUF_SIZE;
    pthread_mutex_unlock(&b->mutex);

    sem_post(&b->full);
}

int buffer_get(Buffer *b) {
    sem_wait(&b->full);

    pthread_mutex_lock(&b->mutex);
    int item = b->dados[b->out];
    b->out = (b->out + 1) % BUF_SIZE;
    pthread_mutex_unlock(&b->mutex);

    sem_post(&b->empty);
    return item;
}

int buffer_count(Buffer *b) {
    int val;

    sem_getvalue(&b->full, &val);

    return val;

}

void buffer_destroy(Buffer *b) {
    sem_destroy(&b->empty);
    sem_destroy(&b->full);
    pthread_mutex_destroy(&b->mutex);
}