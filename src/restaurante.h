#ifndef RESTAURANTE_H
#define RESTAURANTE_H

#include <pthread.h>
#include "buffer.h"
#include "render.h"

typedef struct {
    int id;
    Buffer           *buf_pedidos;
    Buffer           *buf_pratos;
    EstadoRestaurante *estado;      /* partilhado com a thread de render */
} ArgThread;

void *cliente    (void *arg);
void *cozinheiro (void *arg);
void *empregado  (void *arg);

#endif /* RESTAURANTE_H */
