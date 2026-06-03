#ifndef RESTAURANTE_H
#define RESTAURANTE_H

#include <pthread.h>
#include "buffer.h"

typedef struct {

    int id;
    Buffer *buf_pedidos;
    Buffer *buf_pratos;
} ArgThread;

void *cliente(void *arg);

void *cozinheiro(void *arg);

void *empregado(void *arg);

#endif /* RESTAURANTE_H */