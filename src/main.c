#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#include "restaurante.h"
#include "buffer.h"
#include "render.h"

Buffer             buf_pedidos;
Buffer             buf_pratos;
EstadoRestaurante  estado;
pthread_mutex_t    render_lock = PTHREAD_MUTEX_INITIALIZER;

static void init_sistema(void)
{
    buffer_init(&buf_pedidos);
    snprintf(buf_pedidos.nome, 32, "Pedidos");
    buffer_init(&buf_pratos);
    snprintf(buf_pratos.nome, 32, "Pratos");

    memset(&estado, 0, sizeof(estado));
    estado.render_lock = &render_lock;

    /* estados iniciais visíveis na janela */
    for (int i = 0; i < NUM_CLIENTES; i++)
        snprintf(estado.status_Clientes[i], 32, "Cli.%d: inact.", i + 1);
    for (int i = 0; i < NUM_COZINHEIROS; i++)
        snprintf(estado.status_Cozinheiros[i], 32, "Coz.%d: inact.", i + 1);
    for (int i = 0; i < NUM_EMPREGADOS; i++)
        snprintf(estado.status_Empregados[i], 32, "Emp.%d: inact.", i + 1);
}

static void destroy_sistema(void)
{
    buffer_destroy(&buf_pedidos);
    buffer_destroy(&buf_pratos);
    pthread_mutex_destroy(&render_lock);
}

int main(void)
{
    init_sistema();

    /* arrays de argumentos — um por thread */
    ArgThread args_clientes   [NUM_CLIENTES];
    ArgThread args_cozinheiros[NUM_COZINHEIROS];
    ArgThread args_empregados [NUM_EMPREGADOS];

    pthread_t thr_clientes   [NUM_CLIENTES];
    pthread_t thr_cozinheiros[NUM_COZINHEIROS];
    pthread_t thr_empregados [NUM_EMPREGADOS];
    pthread_t thr_render;

    /* thread de visualização — arranca primeiro */
    pthread_create(&thr_render, NULL, thread_render, &estado);

    for (int i = 0; i < NUM_CLIENTES; i++) {
        args_clientes[i] = (ArgThread){
            .id          = i + 1,
            .buf_pedidos = &buf_pedidos,
            .buf_pratos  = &buf_pratos,
            .estado      = &estado
        };
        pthread_create(&thr_clientes[i], NULL, cliente, &args_clientes[i]);
    }

    for (int i = 0; i < NUM_COZINHEIROS; i++) {
        args_cozinheiros[i] = (ArgThread){
            .id          = i + 1,
            .buf_pedidos = &buf_pedidos,
            .buf_pratos  = &buf_pratos,
            .estado      = &estado
        };
        pthread_create(&thr_cozinheiros[i], NULL, cozinheiro, &args_cozinheiros[i]);
    }

    for (int i = 0; i < NUM_EMPREGADOS; i++) {
        args_empregados[i] = (ArgThread){
            .id          = i + 1,
            .buf_pedidos = &buf_pedidos,
            .buf_pratos  = &buf_pratos,
            .estado      = &estado
        };
        pthread_create(&thr_empregados[i], NULL, empregado, &args_empregados[i]);
    }

    /* o programa corre enquanto a janela SDL estiver aberta */
    pthread_join(thr_render, NULL);

    /* ao fechar a janela, cancela todas as threads de negócio */
    for (int i = 0; i < NUM_CLIENTES;    i++) pthread_cancel(thr_clientes[i]);
    for (int i = 0; i < NUM_COZINHEIROS; i++) pthread_cancel(thr_cozinheiros[i]);
    for (int i = 0; i < NUM_EMPREGADOS;  i++) pthread_cancel(thr_empregados[i]);

    for (int i = 0; i < NUM_CLIENTES;    i++) pthread_join(thr_clientes[i],    NULL);
    for (int i = 0; i < NUM_COZINHEIROS; i++) pthread_join(thr_cozinheiros[i], NULL);
    for (int i = 0; i < NUM_EMPREGADOS;  i++) pthread_join(thr_empregados[i],  NULL);

    destroy_sistema();
    return 0;
}
