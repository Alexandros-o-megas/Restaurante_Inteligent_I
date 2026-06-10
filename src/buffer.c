#include "buffer.h"
#include <stdlib.h>
#include "render.h"

void buffer_init(Buffer *b)
{
    b->head = 0;
    b->tail = 0;
    b->count = 0;
    b->total_bloqueios = 0;
    sem_init(&b->empty, 0, BUFFER_SIZE); /* BUFFER_SIZE slots livres */
    sem_init(&b->full, 0, 0);            /* 0 slots cheios           */
    pthread_mutex_init(&b->mutex, NULL);
}

void buffer_destroy(Buffer *b)
{
    sem_destroy(&b->empty);
    sem_destroy(&b->full);
    pthread_mutex_destroy(&b->mutex);
}

/*
 * Tenta sem_trywait primeiro.
 * Se falhar (buffer cheio/vazio), é um bloqueio real — conta e faz sem_wait.
 */
void buffer_put(Buffer *b, int val)
{
    /* tenta sem bloquear */
    if (sem_trywait(&b->empty) != 0)
    {
        /* buffer cheio — vai bloquear */
        char msg[128];
        snprintf(msg, sizeof(msg), "Buffer %s cheio — thread bloqueada", b->nome);
        render_log(msg, LOG_WARN);
        pthread_mutex_lock(&b->mutex);
        b->total_bloqueios++;
        pthread_mutex_unlock(&b->mutex);
        sem_wait(&b->empty); /* bloqueia até haver espaço */
    }

    pthread_mutex_lock(&b->mutex);
    b->data[b->tail] = val;
    b->tail = (b->tail + 1) % BUFFER_SIZE;
    b->count++;
    pthread_mutex_unlock(&b->mutex);

    sem_post(&b->full);
}

int buffer_get(Buffer *b)
{
    /* tenta sem bloquear */
    if (sem_trywait(&b->full) != 0)
    {
        /* buffer vazio — vai bloquear */
        char msg[128];
        snprintf(msg, sizeof(msg), "Buffer %s vazio — thread bloqueada", b->nome);
        render_log(msg, LOG_WARN);
        pthread_mutex_lock(&b->mutex);
        b->total_bloqueios++;
        pthread_mutex_unlock(&b->mutex);
        sem_wait(&b->full); /* bloqueia até haver item */
    }

    pthread_mutex_lock(&b->mutex);
    int val = b->data[b->head];
    b->head = (b->head + 1) % BUFFER_SIZE;
    b->count--;
    pthread_mutex_unlock(&b->mutex);

    sem_post(&b->empty);
    return val;
}

int buffer_count(Buffer *b)
{
    pthread_mutex_lock(&b->mutex);
    int c = b->count;
    pthread_mutex_unlock(&b->mutex);
    return c;
}

void buffer_snapshot(Buffer *b, int *out, int *count)
{
    pthread_mutex_lock(&b->mutex);
    *count = b->count;
    for (int i = 0; i < b->count; i++)
        out[i] = b->data[(b->head + i) % BUFFER_SIZE];
    pthread_mutex_unlock(&b->mutex);
}