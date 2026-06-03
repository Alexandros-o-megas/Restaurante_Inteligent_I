#include "render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void renderizar_texto(SDL_Renderer *ren, TTF_Font *font, const char *texto, int x, int y, SDL_Color cor) {
    SDL_Surface *surf = TTF_RenderText_Blended(font, texto, cor);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(ren, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
/** 
    pthread_mutex_lock(&estado->render_lock);

    printf("\n--- Estado do Restaurante ---\n");
    printf("Pedidos na fila: %d\n", estado->n_pedidos);
    printf("Pratos prontos: %d\n", estado->n_pratos);
    printf("Total entregas: %d\n", estado->total_entregas);
    printf("Total bloqueios: %d\n", estado->total_bloqueios);

    printf("\nStatus dos Clientes:\n");
    for(int i = 0; i < NUM_CLIENTES; i++) {
        printf("Cliente %d: %s\n", i, estado->status_Clientes[i]);
    }

    printf("\nStatus dos Cozinheiros:\n");
    for(int i = 0; i < NUM_COZINHEIROS; i++) {
        printf("Cozinheiro %d: %s\n", i, estado->status_Cozinheiros[i]);
    }

    printf("\nStatus dos Empregados:\n");
    for(int i = 0; i < NUM_EMPREGADOS; i++) {
        printf("Empregado %d: %s\n", i, estado->status_Empregados[i]);
    }

    pthread_mutex_unlock(&estado->render_lock);

*/}

static void desenhar_barra_buffer(SDL_Renderer *ren, TTF_Font *font, const char *nome, int actual, int maximo, int x, int y, SDL_Color cor) {

    SDL_SetRenderDrawColor(ren, 50, 50, 60, 255);
    SDL_Rect fundo = {x, y, 400, 28};
    SDL_RenderFillRect(ren, &fundo);

    int larg = (actual * 400) / maximo;
    SDL_SetRenderDrawColor(ren, cor.r, cor.g, cor.b, cor.a);
    SDL_Rect cheio = { x, y, larg, 28};
    SDL_RenderFillRect(ren, &cheio);

    char label[64];
    snprintf(label, sizeof(label), "%s: %d/%d", nome, actual, maximo);
    SDL_Color cor = {255, 255, 255, 255};
    renderizar_texto(ren, font, label, x + 8, y + 5, branco);
}

