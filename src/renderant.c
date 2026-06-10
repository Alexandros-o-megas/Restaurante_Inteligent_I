#include "render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Utilitário: desenha texto com a fonte TTF                           */
/* ------------------------------------------------------------------ */
static void renderizar_texto(SDL_Renderer *ren, TTF_Font *font,
                              const char *texto, int x, int y,
                              SDL_Color cor)
{
    SDL_Surface *surf = TTF_RenderText_Blended(font, texto, cor);
    if (!surf) return;

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect dst = { x, y, surf->w, surf->h };
    SDL_RenderCopy(ren, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

/* ------------------------------------------------------------------ */
/* Barra de progresso para um buffer                                   */
/* ------------------------------------------------------------------ */
static void desenhar_barra_buffer(SDL_Renderer *ren, TTF_Font *font,
                                  const char *nome, int actual, int maximo,
                                  int x, int y, SDL_Color cor)
{
    /* fundo cinza */
    SDL_SetRenderDrawColor(ren, 50, 50, 60, 255);
    SDL_Rect fundo = { x, y, 400, 28 };
    SDL_RenderFillRect(ren, &fundo);

    /* preenchimento proporcional */
    int larg = (maximo > 0) ? (actual * 400) / maximo : 0;
    SDL_SetRenderDrawColor(ren, cor.r, cor.g, cor.b, cor.a);
    SDL_Rect cheio = { x, y, larg, 28 };
    SDL_RenderFillRect(ren, &cheio);

    /* etiqueta */
    char label[64];
    snprintf(label, sizeof(label), "%s: %d/%d", nome, actual, maximo);
    SDL_Color branco = { 255, 255, 255, 255 };
    renderizar_texto(ren, font, label, x + 8, y + 5, branco);
}

/* ------------------------------------------------------------------ */
/* Card colorido para o estado de uma thread                           */
/* ------------------------------------------------------------------ */
static void desenhar_card(SDL_Renderer *ren, TTF_Font *font,
                          const char *texto, int x, int y,
                          SDL_Color cor)
{
    SDL_SetRenderDrawColor(ren, cor.r, cor.g, cor.b, cor.a);
    SDL_Rect card = { x, y, 150, 40 };
    SDL_RenderFillRect(ren, &card);

    SDL_Color branco = { 255, 255, 255, 255 };
    renderizar_texto(ren, font, texto, x + 6, y + 10, branco);
}

/* ------------------------------------------------------------------ */
/* Desenha o estado completo do restaurante num frame                  */
/* ------------------------------------------------------------------ */
static void desenhar_estado(SDL_Renderer *ren, TTF_Font *font,
                             EstadoRestaurante *e)
{
    SDL_Color cor_pedidos  = {  55, 138, 221, 255 };  /* azul  */
    SDL_Color cor_pratos   = {  29, 158, 117, 255 };  /* verde */
    SDL_Color cor_cliente  = {  90, 130, 200, 255 };
    SDL_Color cor_cozinha  = { 200, 110,  50, 255 };
    SDL_Color cor_empregad = {  80, 170,  80, 255 };
    SDL_Color branco       = { 255, 255, 255, 255 };
    SDL_Color vermelho     = { 255, 100, 100, 255 };

    /* --- Buffers --- */
    desenhar_barra_buffer(ren, font, "Pedidos",
                          e->n_pedidos, 5, 50,  50, cor_pedidos);
    desenhar_barra_buffer(ren, font, "Pratos",
                          e->n_pratos,  5, 50, 100, cor_pratos);

    /* --- Separador --- */
    SDL_SetRenderDrawColor(ren, 60, 60, 70, 255);
    SDL_Rect sep = { 30, 145, 840, 2 };
    SDL_RenderFillRect(ren, &sep);

    /* --- Clientes --- */
    renderizar_texto(ren, font, "Clientes:", 30, 155, branco);
    for (int i = 0; i < NUM_CLIENTES; i++) {
        desenhar_card(ren, font, e->status_Clientes[i],
                      30 + i * 160, 180, cor_cliente);
    }

    /* --- Cozinheiros --- */
    renderizar_texto(ren, font, "Cozinheiros:", 30, 240, branco);
    for (int i = 0; i < NUM_COZINHEIROS; i++) {
        desenhar_card(ren, font, e->status_Cozinheiros[i],
                      30 + i * 160, 265, cor_cozinha);
    }

    /* --- Empregados --- */
    renderizar_texto(ren, font, "Empregados:", 30, 325, branco);
    for (int i = 0; i < NUM_EMPREGADOS; i++) {
        desenhar_card(ren, font, e->status_Empregados[i],
                      30 + i * 160, 350, cor_empregad);
    }

    /* --- Métricas --- */
    char buf[64];
    snprintf(buf, sizeof(buf), "Entregas:  %d", e->total_entregas);
    renderizar_texto(ren, font, buf, 680, 50, branco);
    snprintf(buf, sizeof(buf), "Bloqueios: %d", e->total_bloqueios);
    renderizar_texto(ren, font, buf, 680, 80, vermelho);
}

/* ------------------------------------------------------------------ */
/* Thread principal de renderização SDL                                */
/* ------------------------------------------------------------------ */
void *thread_render(void *arg)
{
    EstadoRestaurante *estado = (EstadoRestaurante *)arg;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return NULL;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return NULL;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Restaurante Inteligente",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        TTF_Quit(); SDL_Quit();
        return NULL;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(
        win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!ren) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        TTF_Quit(); SDL_Quit();
        return NULL;
    }

    TTF_Font *font = TTF_OpenFont(FONT_PATH, 14);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        TTF_Quit(); SDL_Quit();
        return NULL;
    }

    int rodando = 1;
    while (rodando) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) rodando = 0;
        }

        SDL_SetRenderDrawColor(ren, 18, 18, 24, 255);
        SDL_RenderClear(ren);

        pthread_mutex_lock(estado->render_lock);
        desenhar_estado(ren, font, estado);
        pthread_mutex_unlock(estado->render_lock);

        SDL_RenderPresent(ren);
        SDL_Delay(FPS_DELAY);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return NULL;
}
