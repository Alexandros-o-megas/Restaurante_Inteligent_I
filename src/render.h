#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <pthread.h>

#include "buffer.h"

#define WINDOW_WIDTH  900
#define WINDOW_HEIGHT 600
#define FPS_DELAY     33
#define FONT_PATH     "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"

#define NUM_CLIENTES    10
#define NUM_COZINHEIROS  5
#define NUM_EMPREGADOS   3

typedef struct {
    int n_pedidos;          /* itens actuais no buffer de pedidos  */
    int n_pratos;           /* itens actuais no buffer de pratos   */

    char status_Clientes   [NUM_CLIENTES]   [32];
    char status_Cozinheiros[NUM_COZINHEIROS][32];
    char status_Empregados [NUM_EMPREGADOS] [32];

    int total_entregas;
    int total_bloqueios;

    pthread_mutex_t *render_lock;

    int pedidos_buf[BUFFER_SIZE]; //IDs reais no buffer
} EstadoRestaurante;

/* Tipos de entrada no log */
typedef enum { LOG_NORMAL, LOG_WARN, LOG_OK } LogKind;

/* Adiciona uma linha ao log (thread-safe; chamado das threads do restaurante) */
void render_log(const char *msg, LogKind kind);

void *thread_render(void *arg);

#endif /* RENDER_H */
