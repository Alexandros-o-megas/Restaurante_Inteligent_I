#ifndef BUFFER_H
#define BUFFER_H

#include <semaphore.h>
#include <pthread.h>

#define BUFFER_SIZE 5

typedef struct {
    int          data[BUFFER_SIZE];
    int          head, tail, count;
    sem_t        empty;   /* slots livres  */
    sem_t        full;    /* slots cheios  */
    pthread_mutex_t mutex;
    int          total_bloqueios; /* bloqueios detectados neste buffer */
    char         nome[32]; /* para debug */
} Buffer;

void buffer_init   (Buffer *b);
void buffer_destroy(Buffer *b);
void buffer_put    (Buffer *b, int val);
int  buffer_get    (Buffer *b);
int  buffer_count  (Buffer *b);
void buffer_snapshot(Buffer *b, int *out, int *count);

#endif