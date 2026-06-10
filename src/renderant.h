#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <pthread.h>

#define WINDOW_WIDTH  900
#define WINDOW_HEIGHT 600
#define FPS_DELAY     33
#define FONT_PATH     "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"

/* Dimensões do sistema — definidas aqui para serem partilhadas */
#define NUM_CLIENTES    10
#define NUM_COZINHEIROS  5
#define NUM_EMPREGADOS   3

typedef struct {
    int n_pedidos;
    int n_pratos;

    char status_Clientes   [NUM_CLIENTES]   [32];
    char status_Cozinheiros[NUM_COZINHEIROS][32];
    char status_Empregados [NUM_EMPREGADOS] [32];

    int total_entregas;
    int total_bloqueios;

    pthread_mutex_t *render_lock; /* ponteiro — o mutex vive no main */
} EstadoRestaurante;

void *thread_render(void *arg);

#endif /* RENDER_H */
