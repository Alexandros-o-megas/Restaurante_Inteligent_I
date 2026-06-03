#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <pthread.h>
#include "buffer.h"

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600
#define FPS_DELAY 33
#define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"

typedef struct {
    
    int n_pedidos;
    int n_pratos;
    
    char status_Clientes[NUM_CLIENTES][20];
    char status_Cozinheiros[NUM_COZINHEIROS][20];
    char status_Empregados[NUM_EMPREGADOS][20];
    
    int total_entregas;
    int total_bloqueios;

    pthread_mutex_t render_lock;
} EstadoRestaurante;

void *thread_render(void *arg);

#endif /* RENDER_H */