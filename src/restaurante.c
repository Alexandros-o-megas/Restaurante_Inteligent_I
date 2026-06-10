#include "restaurante.h"
#include "render.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Actualiza total_bloqueios no estado somando os dos dois buffers */
static void sync_bloqueios(ArgThread *args)
{
    args->estado->total_bloqueios =
        args->buf_pedidos->total_bloqueios +
        args->buf_pratos->total_bloqueios;
}

void *cliente(void *arg)
{
    ArgThread *args   = (ArgThread *)arg;
    int        id     = args->id;
    int        pedido = id * 100;

    while (1) {
        sleep(1 + rand() % 3);
        pedido++;

        pthread_mutex_lock(args->estado->render_lock);
        snprintf(args->estado->status_Clientes[id - 1], 32,
                 "Cli.%d: pedido", id);
        pthread_mutex_unlock(args->estado->render_lock);

        char msg[128];
        snprintf(msg, sizeof(msg), "Cliente %d fez pedido #%d", id, pedido);
        render_log(msg, LOG_OK);

        buffer_put(args->buf_pedidos, pedido); /* pode bloquear */

        pthread_mutex_lock(args->estado->render_lock);
        snprintf(args->estado->status_Clientes[id - 1], 32,
                 "Cli.%d: espera", id);
        args->estado->n_pedidos = buffer_count(args->buf_pedidos);
        sync_bloqueios(args);
        pthread_mutex_unlock(args->estado->render_lock);
    }
    return NULL;
}

void *cozinheiro(void *arg)
{
    ArgThread *args = (ArgThread *)arg;
    int        id   = args->id;

    while (1) {
        pthread_mutex_lock(args->estado->render_lock);
        snprintf(args->estado->status_Cozinheiros[id - 1], 32,
                 "Coz.%d: aguarda", id);
        pthread_mutex_unlock(args->estado->render_lock);

        int pedido = buffer_get(args->buf_pedidos); /* pode bloquear */

        char msg[128];
        snprintf(msg, sizeof(msg), "Cozinheiro %d preparando prato #%d", id, pedido);
        render_log(msg, LOG_NORMAL);

        pthread_mutex_lock(args->estado->render_lock);
        snprintf(args->estado->status_Cozinheiros[id - 1], 32,
                 "Coz.%d: prep.%d", id, pedido);
        args->estado->n_pedidos = buffer_count(args->buf_pedidos);
        sync_bloqueios(args);
        pthread_mutex_unlock(args->estado->render_lock);

        sleep(2 + rand() % 3);

        buffer_put(args->buf_pratos, pedido); /* pode bloquear */

        snprintf(msg, sizeof(msg), "Cozinheiro %d finalizou prato #%d", id, pedido);
        render_log(msg, LOG_OK);

        pthread_mutex_lock(args->estado->render_lock);
        snprintf(args->estado->status_Cozinheiros[id - 1], 32,
                 "Coz.%d: livre", id);
        args->estado->n_pratos = buffer_count(args->buf_pratos);
        sync_bloqueios(args);
        pthread_mutex_unlock(args->estado->render_lock);
    }
    return NULL;
}

void *empregado(void *arg)
{
    ArgThread *args = (ArgThread *)arg;
    int        id   = args->id;

    while (1) {
        pthread_mutex_lock(args->estado->render_lock);
        snprintf(args->estado->status_Empregados[id - 1], 32,
                 "Emp.%d: aguarda", id);
        pthread_mutex_unlock(args->estado->render_lock);

        int prato = buffer_get(args->buf_pratos); /* pode bloquear */

        char msg[128];
        snprintf(msg, sizeof(msg), "Empregado %d entregou prato #%d", id, prato);
        render_log(msg, LOG_OK);

        pthread_mutex_lock(args->estado->render_lock);
        snprintf(args->estado->status_Empregados[id - 1], 32,
                 "Emp.%d: entr.%d", id, prato);
        args->estado->n_pratos = buffer_count(args->buf_pratos);
        sync_bloqueios(args);
        pthread_mutex_unlock(args->estado->render_lock);

        sleep(1 + rand() % 2);

        pthread_mutex_lock(args->estado->render_lock);
        snprintf(args->estado->status_Empregados[id - 1], 32,
                 "Emp.%d: livre", id);
        args->estado->total_entregas++;
        pthread_mutex_unlock(args->estado->render_lock);
    }
    return NULL;
}