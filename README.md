# Restaurante Inteligente I

Simulação de restaurante com múltiplas threads em C usando sincronização por semáforos, mutexes e fila circular.

## Visão geral

O projeto simula a rotina de um restaurante com:
- Clientes (`cliente`) fazendo pedidos
- Cozinheiros (`cozinheiro`) preparando pratos
- Empregados (`empregado`) entregando pratos
- Renderização / monitoramento do estado do restaurante

A comunicação entre as etapas é feita por buffers compartilhados protegidos por semáforos.

## Estrutura do código

### `src/main.c`

- Inicializa a sincronização entre threads (`sem_t`, `pthread_mutex_t`)
- Cria os buffers de pedidos e pratos
- Cria threads para:
  - `NUM_CLIENTES` clientes
  - `NUM_COZINHEIROS` cozinheiros
  - `NUM_EMPREGADOS` empregados
  - `thread_render` para renderização/monitoramento
- Cancela e aguarda o fim das threads quando a renderização termina

### `src/restaurante.c`

Implementa as rotinas de cada thread:
- `cliente`: gera pedidos periodicamente e coloca no buffer de pedidos
- `cozinheiro`: retira pedidos do buffer de pedidos, prepara e coloca pratos prontos no buffer de pratos
- `empregado`: retira pratos do buffer de pratos e simula a entrega

### `src/buffer.c` e `src/buffer.h`

Implementam um buffer circular seguro para o uso de múltiplas threads:
- `buffer_init` inicializa semáforos e mutexes
- `buffer_put` adiciona um item ao buffer
- `buffer_get` retira um item do buffer
- `buffer_count` retorna a quantidade de itens prontos
- `buffer_destroy` libera recursos

O buffer usa um tamanho fixo de `BUF_SIZE` (5) e garante exclusão mútua e sincronização entre produtores e consumidores.

### `src/render.c` e `src/render.h`

Responsável pela renderização do estado do restaurante com SDL2/TTF.

- `render.h` define constantes de janela e a estrutura `EstadoRestaurante`
- `EstadoRestaurante` armazena contadores de pedidos, pratos, entregas e bloqueios, além de status dos participantes
- `render.c` possui funções utilitárias para desenhar texto e barras de progresso na janela

> Nota: o arquivo `src/render.c` contém partes comentadas relacionadas à impressão de estado no console.

## Dependências

- `gcc`
- `pthread`
- `SDL2`
- `SDL2_ttf`

Em distribuições Debian/Ubuntu, instale com:

```bash
sudo apt update
sudo apt install build-essential libsdl2-dev libsdl2-ttf-dev
```

## Compilação

```bash
gcc -o restaurante src/*.c -lpthread -lSDL2 -lSDL2_ttf
```

## Execução

```bash
./restaurante
```

## Constantes importantes

- `NUM_CLIENTES`: 10
- `NUM_COZINHEIROS`: 5
- `NUM_EMPREGADOS`: 3
- `BUF_SIZE`: 5
- `WINDOW_WIDTH`: 900
- `WINDOW_HEIGHT`: 600
- `FPS_DELAY`: 33
- `FONT_PATH`: `/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf`
