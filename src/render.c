#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "render.h"
#include "types.h"
#include "map.h"
#include "traffic_light.h"

// Dá acesso ao estado global da simulação
extern SimState sim;

// Definição de Cores ANSI apenas para os elementos do mapa
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_WHITE   "\033[1;37m"
#define COLOR_GRAY    "\033[90m"

// Caracteres básicos usados na renderização do mapa
#define CH_WALL   ' '
#define CH_ROAD   '.'
#define CH_CROSS  '+'
#define CH_CAR    'C'
#define CH_AMBU   'A'
#define CH_RED    'R'
#define CH_GREEN  'G'

void render_init(void) {
    printf("\033[?25l"); // Esconde o cursor do terminal
    printf("\033[2J");   // Limpa a tela inteira uma vez no início
}

void render_frame(void) {
    char buf[MAP_ROWS][MAP_COLS]; 

    //Trava o render_lock antes de ler o estado global
    pthread_mutex_lock(&sim.render_lock);

    //Monta a matriz base do mapa (paredes, ruas e semáforos)
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            Cell *cell = &sim.map[r][c];

            if (cell->type == CELL_WALL) {
                buf[r][c] = CH_WALL;
            } 
            else if (cell->type == CELL_ROAD) {
                buf[r][c] = CH_ROAD;
            } 
            else if (cell->type == CELL_INTERSECTION) {
                TrafficLight *light = map_get_light_at(r, c);
                if (light != NULL) {
                    if (light->ns_state == LIGHT_GREEN) {
                        buf[r][c] = CH_GREEN;
                    } else {
                        buf[r][c] = CH_RED;
                    }
                } else {
                    buf[r][c] = CH_CROSS;
                }
            }
        }
    }

    //Sobrepõe os veículos comuns ativos no buffer
    for (int i = 0; i < sim.num_vehicles; i++) {
        Vehicle *v = &sim.vehicles[i];
        if (v->active && v->type != AMBULANCE) {
            buf[v->row][v->col] = CH_CAR;
        }
    }

    //Sobrepõe a ambulância por cima de tudo para garantir prioridade visual
    for (int i = 0; i < sim.num_vehicles; i++) {
        Vehicle *v = &sim.vehicles[i];
        if (v->active && v->type == AMBULANCE) {
            buf[v->row][v->col] = CH_AMBU;
        }
    }

    //ATUALIZADO: Em vez de apenas mover o cursor, vamos usar \033[2J antes de 
    // mover para o topo \033[H. Isso apaga qualquer texto fantasma que ficou nos lados!
    printf("\033[2J\033[H");

    //Varre a matriz aplicando cores ANSI estritamente no mapa
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            char ch = buf[r][c];
            
            if (ch == CH_AMBU) {
                printf("%s%c%s", COLOR_CYAN, ch, COLOR_RESET);
            } else if (ch == CH_CAR) {
                printf("%s%c%s", COLOR_WHITE, ch, COLOR_RESET);
            } else if (ch == CH_GREEN) {
                printf("%s%c%s", COLOR_GREEN, ch, COLOR_RESET);
            } else if (ch == CH_RED) {
                printf("%s%c%s", COLOR_RED, ch, COLOR_RESET);
            } else if (ch == CH_ROAD) {
                printf("%s%c%s", COLOR_GRAY, ch, COLOR_RESET);
            } else if (ch == CH_CROSS) {
                printf("%s%c%s", COLOR_YELLOW, ch, COLOR_RESET);
            } else {
                printf("%c", ch); // Espaço em branco da parede
            }
        }
        printf("\n");
    }

    //Exibe o painel de status de forma estritamente linear e sem cores
    printf("\nSTATUS DA SIMULACAO\n");
    printf("Tempo (Tick): %ld\n", sim.tick);

    //Descarrega o buffer de saída imediatamente
    fflush(stdout);

    //Libera o lock para o fluxo concorrente continuar
    pthread_mutex_unlock(&sim.render_lock);
}