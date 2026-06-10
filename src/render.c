/*
 * render.c  –  Interface SDL2 inspirada no simulador HTML
 *
 * Layout (900 x 600):
 *   [Metricas: 4 cards]
 *   [Buf.Pedidos: 5 quadrados] [Buf.Pratos: 5 quadrados]
 *   [Col Clientes | Col Buffer Pedidos | Col Cozinheiros | Col Empregados]
 *   [Log de eventos]
 *
 * Scroll nas colunas via roda do rato.
 */

#include "render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ── Paleta ──────────────────────────────────────────────────────── */
#define COL_BG        { 18,  18,  24, 255}
#define COL_BG2       { 30,  30,  40, 255}
#define COL_BORDER    { 50,  50,  65, 255}
#define COL_TEXT      {220, 220, 228, 255}
#define COL_MUTED     {130, 130, 148, 255}
#define COL_BLUE      { 55, 138, 221, 255}
#define COL_GREEN     { 99, 153,  34, 255}
#define COL_ORANGE    {239, 159,  39, 255}
#define COL_RED       {220,  80,  80, 255}
#define COL_IDLE      { 70,  70,  85, 255}
#define COL_OK        {100, 200, 120, 255}

/* ── Log circular ────────────────────────────────────────────────── */
#define LOG_LINES 120
#define LOG_VIS   10

typedef struct { char text[128]; LogKind kind; } LogLine;

static LogLine         g_log[LOG_LINES];
static int             g_log_head  = 0;
static int             g_log_count = 0;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

void render_log(const char *msg, LogKind kind)
{
    time_t t   = time(NULL);
    struct tm *tm = localtime(&t);
    pthread_mutex_lock(&g_log_mutex);
    snprintf(g_log[g_log_head].text, sizeof(g_log[0].text),
             "[%02d:%02d:%02d] %s", tm->tm_hour, tm->tm_min, tm->tm_sec, msg);
    g_log[g_log_head].kind = kind;
    g_log_head  = (g_log_head + 1) % LOG_LINES;
    if (g_log_count < LOG_LINES) g_log_count++;
    pthread_mutex_unlock(&g_log_mutex);
}

/* ── Scroll das colunas (offset em linhas, alterado pelo rato) ───── */
static int g_scroll_cli = 0;
static int g_scroll_buf = 0;
static int g_scroll_coz = 0;
static int g_scroll_emp = 0;

/* largura/posicao das colunas — preenchidas em draw_frame, lidas no handler */
static int g_col_x[4], g_col_w[4], g_col_y, g_col_h;

static void handle_scroll(int mx, int my, int dy)
{
    for (int c = 0; c < 4; c++) {
        if (mx >= g_col_x[c] && mx < g_col_x[c] + g_col_w[c] &&
            my >= g_col_y    && my < g_col_y    + g_col_h) {
            int *ptrs[4] = {&g_scroll_cli,&g_scroll_buf,&g_scroll_coz,&g_scroll_emp};
            int *sc = ptrs[c];
            *sc -= dy;
            if (*sc < 0) *sc = 0;
        }
    }
}

/* ── Primitivas ──────────────────────────────────────────────────── */
static SDL_Color mkc(Uint8 r,Uint8 g,Uint8 b,Uint8 a)
    { SDL_Color c={r,g,b,a}; return c; }

static void fill_rect(SDL_Renderer *ren, SDL_Color c, int x,int y,int w,int h)
{
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
    SDL_Rect r={x,y,w,h};
    SDL_RenderFillRect(ren,&r);
}

static void draw_rect(SDL_Renderer *ren, SDL_Color c, int x,int y,int w,int h)
{
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
    SDL_Rect r={x,y,w,h};
    SDL_RenderDrawRect(ren,&r);
}

static void render_text(SDL_Renderer *ren, TTF_Font *font,
                         const char *txt, int x,int y, SDL_Color col)
{
    if (!txt || !*txt) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended(font, txt, col);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(ren, s);
    SDL_Rect dst = {x, y, s->w, s->h};
    SDL_RenderCopy(ren, t, NULL, &dst);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(s);
}

static void fill_circle(SDL_Renderer *ren, SDL_Color c, int cx,int cy,int r)
{
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
    for (int dy=-r; dy<=r; dy++)
        for (int dx=-r; dx<=r; dx++)
            if (dx*dx+dy*dy <= r*r)
                SDL_RenderDrawPoint(ren, cx+dx, cy+dy);
}

/* ── Scroll bar fina no lado direito da coluna ───────────────────── */
static void draw_scrollbar(SDL_Renderer *ren,
                            int x,int y,int h,
                            int total_items, int visible_items, int offset)
{
    if (total_items <= visible_items) return;
    SDL_Color track = (SDL_Color)COL_BORDER;
    SDL_Color thumb = (SDL_Color)COL_MUTED;
    int sw = 4;
    fill_rect(ren, track, x, y, sw, h);
    int thumb_h = (visible_items * h) / total_items;
    if (thumb_h < 12) thumb_h = 12;
    int thumb_y = y + (offset * (h - thumb_h)) / (total_items - visible_items);
    fill_rect(ren, thumb, x, thumb_y, sw, thumb_h);
}

/* ── Card de metrica ─────────────────────────────────────────────── */
static void draw_metric_card(SDL_Renderer *ren, TTF_Font *fnt, TTF_Font *fnt_big,
                              const char *label, int value,
                              int x,int y,int w,int h, SDL_Color val_col)
{
    SDL_Color bg     = (SDL_Color)COL_BG2;
    SDL_Color border = (SDL_Color)COL_BORDER;
    SDL_Color muted  = (SDL_Color)COL_MUTED;
    fill_rect(ren, bg, x,y,w,h);
    draw_rect(ren, border, x,y,w,h);
    render_text(ren, fnt, label, x+12, y+10, muted);
    char buf[16]; snprintf(buf,sizeof(buf),"%d",value);
    render_text(ren, fnt_big, buf, x+12, y+30, val_col);
}

/* ── Buffer como N quadrados coloridos ───────────────────────────── */
static void draw_buffer_squares(SDL_Renderer *ren, TTF_Font *fnt,
                                 const char *label, int cur, int max,
                                 int x,int y,int w, SDL_Color fill_col)
{
    SDL_Color muted  = (SDL_Color)COL_MUTED;
    SDL_Color bg_sq  = (SDL_Color)COL_BG2;
    SDL_Color border = (SDL_Color)COL_BORDER;
    SDL_Color white  = (SDL_Color)COL_TEXT;

    render_text(ren, fnt, label, x, y+3, muted);

    /* area dos quadrados */
    int tx = x + 100;
    int sq_gap = 4;
    int sq_w = (w - 140 - (max-1)*sq_gap) / max;
    if (sq_w < 8) sq_w = 8;
    int sq_h = 18;

    for (int i = 0; i < max; i++) {
        int sx = tx + i*(sq_w + sq_gap);
        if (i < cur) {
            fill_rect(ren, fill_col, sx, y, sq_w, sq_h);
        } else {
            fill_rect(ren, bg_sq,    sx, y, sq_w, sq_h);
            draw_rect(ren, border,   sx, y, sq_w, sq_h);
        }
    }

    /* contagem */
    char buf[16]; snprintf(buf,sizeof(buf),"%d/%d",cur,max);
    int count_x = tx + max*(sq_w+sq_gap) - sq_gap + 8;
    render_text(ren, fnt, buf, count_x, y+2, white);
}

/* ── Coluna com scroll ───────────────────────────────────────────── */
/* Renderiza items[offset..] dentro da area visivel da coluna.
   Usa SDL_RenderSetClipRect para cortar o conteudo que sai. */
static void draw_column(SDL_Renderer *ren, TTF_Font *fnt,
                         const char *title,
                         const char **items, const char **states, int n,
                         int x,int y,int w,int h,
                         int *scroll)
{
    SDL_Color bg     = (SDL_Color)COL_BG2;
    SDL_Color border = (SDL_Color)COL_BORDER;
    SDL_Color muted  = (SDL_Color)COL_MUTED;
    SDL_Color white  = (SDL_Color)COL_TEXT;

    const int HEADER = 28;
    const int ROW_H  = 22;
    int content_h    = h - HEADER;
    int visible_rows = content_h / ROW_H;

    /* clamp scroll */
    int max_scroll = n - visible_rows;
    if (max_scroll < 0) max_scroll = 0;
    if (*scroll > max_scroll) *scroll = max_scroll;

    fill_rect(ren, bg, x,y,w,h);
    draw_rect(ren, border, x,y,w,h);
    render_text(ren, fnt, title, x+8, y+8, muted);
    fill_rect(ren, border, x+4, y+HEADER-4, w-8, 1);

    /* clip ao interior da coluna (abaixo do cabecalho) */
    SDL_Rect clip = { x+1, y+HEADER, w-2, content_h };
    SDL_RenderSetClipRect(ren, &clip);

    for (int i = *scroll; i < n; i++) {
        int iy = y + HEADER + (i - *scroll) * ROW_H;
        if (iy >= y + h) break;

        SDL_Color dot = (SDL_Color)COL_IDLE;
        if (states && states[i]) {
            if      (strcmp(states[i],"active" )==0) dot = (SDL_Color)COL_GREEN;
            else if (strcmp(states[i],"waiting")==0) dot = (SDL_Color)COL_ORANGE;
        }
        fill_circle(ren, dot, x+12, iy+8, 4);
        render_text(ren, fnt, items ? items[i] : "", x+22, iy+2, white);
    }

    SDL_RenderSetClipRect(ren, NULL);

    /* scrollbar */
    draw_scrollbar(ren, x+w-6, y+HEADER, content_h, n, visible_rows, *scroll);
}

/* ── Coluna do buffer de pedidos com scroll ──────────────────────── */
static void draw_buffer_column(SDL_Renderer *ren, TTF_Font *fnt,
                                const char *title,
                                int *pedidos, int n,
                                int x,int y,int w,int h,
                                int *scroll)
{
    SDL_Color bg      = (SDL_Color)COL_BG2;
    SDL_Color border  = (SDL_Color)COL_BORDER;
    SDL_Color muted   = (SDL_Color)COL_MUTED;
    SDL_Color blue    = (SDL_Color)COL_BLUE;
    SDL_Color info_bg = mkc(30,60,100,255);

    const int HEADER = 28;
    const int ROW_H  = 22;
    int content_h    = h - HEADER;
    int visible_rows = content_h / ROW_H;

    int max_scroll = n - visible_rows;
    if (max_scroll < 0) max_scroll = 0;
    if (*scroll > max_scroll) *scroll = max_scroll;

    fill_rect(ren, bg, x,y,w,h);
    draw_rect(ren, border, x,y,w,h);
    render_text(ren, fnt, title, x+8, y+8, muted);
    fill_rect(ren, border, x+4, y+HEADER-4, w-8, 1);

    SDL_Rect clip = { x+1, y+HEADER, w-2, content_h };
    SDL_RenderSetClipRect(ren, &clip);

    for (int i = *scroll; i < n; i++) {
        int iy = y + HEADER + (i - *scroll) * ROW_H;
        if (iy >= y + h) break;
        fill_rect(ren, info_bg, x+4, iy, w-10, 18);
        char buf[32]; snprintf(buf,sizeof(buf),"Pedido #%d", pedidos[i]);
        render_text(ren, fnt, buf, x+10, iy+2, blue);
    }

    SDL_RenderSetClipRect(ren, NULL);
    draw_scrollbar(ren, x+w-6, y+HEADER, content_h, n, visible_rows, *scroll);
}

/* ── Log de eventos ──────────────────────────────────────────────── */
static void draw_log(SDL_Renderer *ren, TTF_Font *fnt,
                     int x,int y,int w,int h)
{
    SDL_Color bg     = (SDL_Color)COL_BG2;
    SDL_Color border = (SDL_Color)COL_BORDER;
    SDL_Color normal = (SDL_Color)COL_MUTED;
    SDL_Color warn   = (SDL_Color)COL_RED;
    SDL_Color ok     = (SDL_Color)COL_OK;

    fill_rect(ren, bg, x,y,w,h);
    draw_rect(ren, border, x,y,w,h);

    pthread_mutex_lock(&g_log_mutex);

    int vis   = (g_log_count < LOG_VIS) ? g_log_count : LOG_VIS;
    int lh    = h / (LOG_VIS + 1);   /* altura por linha */
    if (lh < 12) lh = 12;
    int start = (g_log_head - vis + LOG_LINES) % LOG_LINES;

    for (int i = 0; i < vis; i++) {
        int idx = (start + i) % LOG_LINES;
        int ty  = y + 4 + i * lh;
        if (ty + lh > y + h) break;  /* nao sair da caixa */
        SDL_Color col = normal;
        if      (g_log[idx].kind == LOG_WARN) col = warn;
        else if (g_log[idx].kind == LOG_OK  ) col = ok;
        render_text(ren, fnt, g_log[idx].text, x+6, ty, col);
    }

    pthread_mutex_unlock(&g_log_mutex);
}

/* ── Frame completo ──────────────────────────────────────────────── */
static void draw_frame(SDL_Renderer *ren,
                        TTF_Font *fnt_sm, TTF_Font *fnt_big,
                        EstadoRestaurante *e)
{
    const int PAD  = 12;
    const int W    = WINDOW_WIDTH;

    SDL_Color text  = (SDL_Color)COL_TEXT;
    SDL_Color red   = (SDL_Color)COL_RED;
    SDL_Color blue  = (SDL_Color)COL_BLUE;
    SDL_Color green = (SDL_Color)COL_GREEN;

    int y = PAD;

    /* ── 1. Metricas ─────────────────────────────────────────── */
    const int MH = 58, MGAP = 8;
    int mw = (W - 2*PAD - 3*MGAP) / 4;
    struct { const char *label; int val; SDL_Color col; } metrics[] = {
        {"Pedidos feitos",  e->total_entregas + e->n_pedidos, text},
        {"Pratos prontos",  e->n_pratos,                      text},
        {"Entregas",        e->total_entregas,                 text},
        {"Bloqueios",       e->total_bloqueios,                red },
    };
    for (int i=0; i<4; i++) {
        int mx = PAD + i*(mw+MGAP);
        draw_metric_card(ren, fnt_sm, fnt_big,
                         metrics[i].label, metrics[i].val,
                         mx, y, mw, MH, metrics[i].col);
    }
    y += MH + MGAP;

    /* ── 2. Buffers como quadrados ───────────────────────────── */
    draw_buffer_squares(ren, fnt_sm, "Buf. Pedidos",
                        e->n_pedidos, 5, PAD, y, W-2*PAD, blue);
    y += 26;
    draw_buffer_squares(ren, fnt_sm, "Buf. Pratos",
                        e->n_pratos,  5, PAD, y, W-2*PAD, green);
    y += 30;

    /* ── 3. Quatro colunas com scroll ────────────────────────── */
    const int CH = 210, CGAP = 8;
    int cw = (W - 2*PAD - 3*CGAP) / 4;

    /* guardar geometria para o handler de scroll */
    g_col_y = y; g_col_h = CH;
    for (int c=0; c<4; c++) {
        g_col_x[c] = PAD + c*(cw+CGAP);
        g_col_w[c] = cw;
    }

    /* Clientes */
    {
        const char *labels[NUM_CLIENTES];
        const char *states[NUM_CLIENTES];
        for (int i=0; i<NUM_CLIENTES; i++) {
            labels[i] = e->status_Clientes[i];
            if      (strstr(e->status_Clientes[i],"bloqueado")) states[i]="waiting";
            else if (strstr(e->status_Clientes[i],"pedido"   )) states[i]="active";
            else                                                  states[i]="idle";
        }
        draw_column(ren, fnt_sm, "CLIENTES",
                    labels, states, NUM_CLIENTES,
                    g_col_x[0], y, cw, CH, &g_scroll_cli);
    }

    /* Buffer de pedidos */
    {
        static int dummy[NUM_CLIENTES];
        for (int i=0; i<e->n_pedidos && i<NUM_CLIENTES; i++) dummy[i]=i+1;
        draw_buffer_column(ren, fnt_sm, "BUFFER PEDIDOS",
                           dummy, e->n_pedidos,
                           g_col_x[1], y, cw, CH, &g_scroll_buf);
    }

    /* Cozinheiros */
    {
        const char *labels[NUM_COZINHEIROS];
        const char *states[NUM_COZINHEIROS];
        for (int i=0; i<NUM_COZINHEIROS; i++) {
            labels[i] = e->status_Cozinheiros[i];
            if      (strstr(e->status_Cozinheiros[i],"aguardando")) states[i]="waiting";
            else if (strstr(e->status_Cozinheiros[i],"preparando")) states[i]="active";
            else                                                       states[i]="idle";
        }
        draw_column(ren, fnt_sm, "COZINHEIROS",
                    labels, states, NUM_COZINHEIROS,
                    g_col_x[2], y, cw, CH, &g_scroll_coz);
    }

    /* Empregados */
    {
        const char *labels[NUM_EMPREGADOS];
        const char *states[NUM_EMPREGADOS];
        for (int i=0; i<NUM_EMPREGADOS; i++) {
            labels[i] = e->status_Empregados[i];
            if      (strstr(e->status_Empregados[i],"aguardando")) states[i]="waiting";
            else if (strstr(e->status_Empregados[i],"entrega"   )) states[i]="active";
            else                                                      states[i]="idle";
        }
        draw_column(ren, fnt_sm, "EMPREGADOS",
                    labels, states, NUM_EMPREGADOS,
                    g_col_x[3], y, cw, CH, &g_scroll_emp);
    }
    y += CH + CGAP;

    /* ── 4. Log ──────────────────────────────────────────────── */
    int log_h = WINDOW_HEIGHT - y - PAD;
    if (log_h > 20)
        draw_log(ren, fnt_sm, PAD, y, W-2*PAD, log_h);
}

/* ── Thread de renderizacao ──────────────────────────────────────── */
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
    if (!win) { TTF_Quit(); SDL_Quit(); return NULL; }

    SDL_Renderer *ren = SDL_CreateRenderer(
        win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!ren) { SDL_DestroyWindow(win); TTF_Quit(); SDL_Quit(); return NULL; }

    TTF_Font *fnt_sm  = TTF_OpenFont(FONT_PATH, 12);
    TTF_Font *fnt_big = TTF_OpenFont(FONT_PATH, 22);
    if (!fnt_sm || !fnt_big) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        if (fnt_sm)  TTF_CloseFont(fnt_sm);
        if (fnt_big) TTF_CloseFont(fnt_big);
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        TTF_Quit(); SDL_Quit();
        return NULL;
    }

    render_log("Sistema de renderizacao iniciado", LOG_OK);

    SDL_Color bg = (SDL_Color)COL_BG;
    int rodando  = 1;

    while (rodando) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                rodando = 0;
            } else if (ev.type == SDL_MOUSEWHEEL) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                handle_scroll(mx, my, ev.wheel.y);
            }
        }

        SDL_SetRenderDrawColor(ren, bg.r, bg.g, bg.b, bg.a);
        SDL_RenderClear(ren);

        pthread_mutex_lock(estado->render_lock);
        draw_frame(ren, fnt_sm, fnt_big, estado);
        pthread_mutex_unlock(estado->render_lock);

        SDL_RenderPresent(ren);
        SDL_Delay(FPS_DELAY);
    }

    TTF_CloseFont(fnt_sm);
    TTF_CloseFont(fnt_big);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return NULL;
}