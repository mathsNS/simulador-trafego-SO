#include <string.h>
#include "map.h"
#include "traffic_light.h"
#include "types.h"

// mapa 11x11
// ruas horizontais: linhas 1, 5, 9
// ruas verticais: colunas 1, 5, 9
// linha 1: mao unica leste | linhas 5 e 9: dupla mao
// 9 cruzamentos nas intersecoes (linha x coluna)

void map_init(void) {
    // TODO: inicializar todas as celulas como CELL_WALL com occupant = -1
    // TODO: setar celulas das ruas horizontais com allowed correto por linha
    //       linha 1: DIR_E (mao unica) | linhas 5 e 9: DIR_E | DIR_W
    // TODO: setar celulas das ruas verticais: allowed |= DIR_N | DIR_S
    // TODO: marcar intersecoes como CELL_INTERSECTION
    // TODO: pthread_mutex_init em cada celula
    // TODO: inicializar sim.tick=0, sim.running=1, mutexes e cond vars globais
}

int map_valid_move(int row, int col, Direction dir, int *next_row, int *next_col) {
    // TODO: calcular celula destino com base em dir (dr/dc)
    // TODO: checar bounds, checar se destino nao e CELL_WALL
    // TODO: checar se o bit de dir esta em sim.map[row][col].allowed
    // retorna 1 se valido e preenche next_row/next_col, 0 caso contrario
    (void)row; (void)col; (void)dir; (void)next_row; (void)next_col;
    return 0;
}

TrafficLight *map_get_light_at(int row, int col) {
    // TODO: percorrer sim.lights e retornar ponteiro para o que tiver row/col iguais
    (void)row; (void)col;
    return NULL;
}
