#ifndef MAP_H
#define MAP_H

#include "types.h"

// inicializa o mapa e todos os mutexes das celulas
void map_init(void);

// retorna 1 se o movimento de (row,col) na direcao dir e valido
// preenche next_row e next_col com a celula destino
int map_valid_move(int row, int col, Direction dir, int *next_row, int *next_col);

// retorna o semaforo do cruzamento em (row,col), NULL se nao for cruzamento
TrafficLight *map_get_light_at(int row, int col);

#endif
