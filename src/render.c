#include <stdio.h>
#include "render.h"
#include "types.h"

// chars usados no mapa
#define CH_WALL   ' '
#define CH_ROAD   '.'
#define CH_CROSS  '+'
#define CH_CAR    'C'
#define CH_AMBU   'A'
#define CH_RED    'R'
#define CH_GREEN  'G'

void render_init(void) {
    printf("\033[?25l"); // esconde cursor
    printf("\033[2J");   // limpa tela uma vez so no inicio
}

void render_frame(void) {
    // TODO: travar render_lock antes de ler o estado
    // TODO: montar um buffer de chars baseado em sim.map
    //       celula WALL     -> CH_WALL
    //       celula ROAD     -> CH_ROAD
    //       celula INTERSECTION -> estado do semaforo (CH_RED ou CH_GREEN)
    // TODO: sobrepor veiculos: percorrer sim.vehicles, se active == 1 e type != AMBULANCE
    //       colocar CH_CAR na posicao (v->row, v->col)
    // TODO: sobrepor ambulancia com CH_AMBU
    // TODO: mover cursor pro topo com \033[H e imprimir o buffer
    // TODO: imprimir tick e status embaixo do mapa
    // TODO: fflush(stdout)
    // TODO: soltar render_lock

    // esqueleto basico pra referencia:
    //
    // char buf[MAP_ROWS][MAP_COLS + 2]; // +2 pra \n e \0
    //
    // pthread_mutex_lock(&sim.render_lock);
    // ... preenche buf ...
    // printf("\033[H");
    // for (int r = 0; r < MAP_ROWS; r++) printf("%s\n", buf[r]);
    // printf("tick: %ld\n", sim.tick);
    // fflush(stdout);
    // pthread_mutex_unlock(&sim.render_lock);
}
