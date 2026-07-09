#include "map.h"
#include "traffic_light.h"
#include "types.h"

// mapa 11x11
// ruas horizontais: linhas 1, 5, 9
// ruas verticais: colunas 1, 5, 9
// linha 1: mao unica leste | linhas 5 e 9: dupla mao
// 9 cruzamentos nas intersecoes (linha x coluna)
//
// estrategia contra deadlock: cada veiculo adquire no maximo um lock de celula
// por vez. antes de tentar o lock da celula destino, libera o da celula atual.
// isso elimina a condicao "hold and wait": nenhuma thread segura um recurso
// enquanto espera por outro, tornando deadlock impossivel.

static const int road_rows[] = {1, 5, 9};
static const int road_cols[] = {1, 5, 9};

void map_init(void) {
    // sim e variavel global, ja zero-inicializada pelo C
    // inicia apenas os campos que precisam de valor especifico

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            Cell *cell  = &sim.map[r][c];
            cell->type     = CELL_WALL;
            cell->allowed  = 0;
            cell->occupant = -1;
            pthread_mutex_init(&cell->lock, NULL);
        }
    }

    for (int i = 0; i < 3; i++) {
        int r = road_rows[i];
        for (int c = 1; c < MAP_COLS - 1; c++) {
            Cell *cell = &sim.map[r][c];
            cell->type    = CELL_ROAD;
            cell->allowed = (r == 1) ? DIR_E : DIR_E | DIR_W;
        }
    }

    for (int j = 0; j < 3; j++) {
        int c = road_cols[j];
        for (int r = 1; r < MAP_ROWS - 1; r++) {
            sim.map[r][c].type     = CELL_ROAD;
            sim.map[r][c].allowed |= DIR_N | DIR_S;
        }
    }

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            sim.map[road_rows[i]][road_cols[j]].type = CELL_INTERSECTION;

    sim.tick         = 0;
    sim.running      = 1;
    sim.num_vehicles = 0;
    pthread_mutex_init(&sim.tick_lock, NULL);
    pthread_cond_init(&sim.tick_cond, NULL);
    pthread_mutex_init(&sim.render_lock, NULL);
}

int map_valid_move(int row, int col, Direction dir, int *next_row, int *next_col) {
    int dr = 0, dc = 0, flag = 0;

    switch (dir) {
        case NORTH: dr = -1; flag = DIR_N; break;
        case SOUTH: dr =  1; flag = DIR_S; break;
        case EAST:  dc =  1; flag = DIR_E; break;
        case WEST:  dc = -1; flag = DIR_W; break;
    }

    int nr = row + dr;
    int nc = col + dc;

    if (nr < 0 || nr >= MAP_ROWS || nc < 0 || nc >= MAP_COLS)
        return 0;
    if (sim.map[nr][nc].type == CELL_WALL)
        return 0;
    if (!(sim.map[row][col].allowed & flag))
        return 0;

    *next_row = nr;
    *next_col = nc;
    return 1;
}

TrafficLight *map_get_light_at(int row, int col) {
    for (int i = 0; i < NUM_LIGHTS; i++) {
        if (sim.lights[i].row == row && sim.lights[i].col == col)
            return &sim.lights[i];
    }
    return NULL;
}
