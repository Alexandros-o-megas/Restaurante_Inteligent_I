#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "restaurante.h"
#include "buffer.h"
#include "render.h"

#define NUM_CLIENTES 10
#define NUM_COZINHEIROS 5
#define NUM_EMPREGADOS 3

sem_t empty_pedidos;
sem_t full_pedidos;
sem_t empty_pratos;
sem_t full_pratos; 
Buffer buf_pedidos; 
Buffer buf_pratos;
pthread_mutex_t mutex_pedidos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pratos = PTHREAD_MUTEX_INITIALIZER;

EstadoRestaurante estado;
pthread_mutex_t render_lock = PTHREAD_MUTEX_INITIALIZER;

static void init_sicronizacao(void) {
    sem_init(&empty_pedidos, 0, BUFFER_SIZE);
    sem_init(&full_pedidos, 0, 0);
    sem_init(&empty_pratos, 0, BUFFER_SIZE);
    sem_init(&full_pratos, 0, 0);

    buffer_init(&buf_pedidos);
    buffer_init(&buf_pratos);

    estado.render_lock = &render_lock;
    estado.n_pedidos = 0;
    estado.n_pratos = 0;
    estado.total_entregues = 0;
    estado.total_bloqueiados = 0;

}

static void destroy_sicronizacao(void) {
    sem_destroy(&empty_pedidos);
    sem_destroy(&full_pedidos);
    sem_destroy(&empty_pratos);
    sem_destroy(&full_pratos);

    buffer_destroy(&buf_pedidos);
    buffer_destroy(&buf_pratos);

    pthread_mutex_destroy(&mutex_pedidos);
    pthread_mutex_destroy(&mutex_pratos);
    pthread_mutex_destroy(&render_lock);

}

int main() {
    init_sicronizacao();

    pthread_t thr_clientes[NUM_CLIENTES];
    pthread_t thr_cozinheiros[NUM_COZINHEIROS];
    pthread_t thr_empregados[NUM_EMPREGADOS];
    pthread_t thr_render;

    int ids[NUM_CLIENTES+NUM_COZINHEIROS+NUM_EMPREGADOS];

    pthread_create(&thr_render, NULL, thread_render, &estado);

    for (int i = 0; i < NUM_CLIENTES; i++) {
        ids[i] = i+1;
        pthread_create(&thr_clientes[i], NULL, cliente, &ids[i]);
    }

    for (int i = 0; i < NUM_COZINHEIROS; i++) {
        ids[NUM_CLIENTES + i] =i + 1;
        pthread_create(&thr_cozinheiros[i], NULL, cozinheiro, &ids[NUM_CLIENTES + i]);
    }

    for (int i = 0; i < NUM_EMPREGADOS; i++) {
        ids[NUM_CLIENTES + NUM_COZINHEIROS + i] =i + 1;
        pthread_create(&thr_empregados[i], NULL, empregado, &ids[NUM_CLIENTES + NUM_COZINHEIROS + i]);
    }

    pthread_join(thr_render, NULL);

    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_cancel(thr_clientes[i]);
    }
    for (int i = 0; i < NUM_COZINHEIROS; i++) {
        pthread_cancel(thr_cozinheiros[i]);
    }
    for (int i = 0; i < NUM_EMPREGADOS; i++) {
        pthread_cancel(thr_empregados[i]);
    }

    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_join(thr_clientes[i], NULL);
    }
    for (int i = 0; i < NUM_COZINHEIROS; i++) {
        pthread_join(thr_cozinheiros[i], NULL);
    }
    for (int i = 0; i < NUM_EMPREGADOS; i++) {
        pthread_join(thr_empregados[i], NULL);
    }
    

    destroy_sicronizacao();
    return 0;
}