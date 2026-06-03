#include "restaurante.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *cliente(void *arg) {
    ArgThread *args = (ArgThread *)arg;
    int id = args->id;
    int pedido_id = id*100;

    while(1) {
        sleep(1 + rand() % 3); // Simular tempo entre pedidos

        pedido_id++;
        printf("Cliente %d fez pedido %d\n", id, pedido_id);
        buffer_put(args->buf_pedidos, pedido_id);
    
    }
    return NULL;
}

void *cozinheiro(void *arg) {
    ArgThread *args = (ArgThread *)arg;
    int id = args->id;

    while(1) {
        int pedido_id = buffer_get(args->buf_pedidos);
        printf("Cozinheiro %d a preparar pedido %d\n", id, pedido_id);
        sleep(2 + rand() % 3); // Simular tempo de preparo

        buffer_put(args->buf_pratos, pedido_id);
        printf("Cozinheiro %d pranto pronto! %d\n", id, pedido_id);
        
    }
    return NULL;
}

void *empregado(void *arg) {
    ArgThread *args = (ArgThread *)arg;
    int id = args->id;

    while(1) {
        int prato_id = buffer_get(args->buf_pratos);
        printf("Empregado %d a entregar prato %d\n", id, prato_id);
        sleep(1 + rand() % 2); // Simular tempo de entrega

        printf("Empregado %d entregou prato %d\n", id, prato_id);
    }
    return NULL;
}