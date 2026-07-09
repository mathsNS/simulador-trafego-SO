#include <stdlib.h>
#include "route.h"
#include "map.h"
#include "types.h"

static const struct {
    int row, col;
    Direction dir;
} entry_points[NUM_ENTRY_POINTS] = {
    {1, 1, EAST},               // linha 1, mão unica
    {5, 1, EAST}, {5, 9, WEST}, // linha 5, mão dupla
    {9, 1, EAST}, {9, 9, WEST}, // linha 9, mão dupla
    {2, 1, SOUTH}, {8, 1, NORTH}, // coluna 1
    {2, 5, SOUTH}, {8, 5, NORTH}, // coluna 5
    {2, 9, SOUTH}, {8, 9, NORTH}, // coluna 9
};

Direction route_dir_between(int row, int col, int next_row, int next_col) {
    if (next_row == row - 1) return NORTH;
    if (next_row == row + 1) return SOUTH;
    if (next_col == col + 1) return EAST;
    return WEST;
}

static Direction opposite_dir(Direction d) {
    switch (d) {
        case NORTH: return SOUTH;
        case SOUTH: return NORTH;
        case EAST:  return WEST;
        case WEST:  return EAST;
    }
    return NORTH;
}

int route_generate_random(int start_row, int start_col, Direction start_dir,
                           int min_len, int out[][2], int *out_len) {
    int row = start_row, col = start_col;
    Direction cur_dir = start_dir;
    int len = 0;

    out[len][0] = row;
    out[len][1] = col;
    len++;

    if (min_len > MAX_ROUTE) min_len = MAX_ROUTE;

    while (len < min_len && len < MAX_ROUTE) {
        Direction all_dirs[4] = {NORTH, SOUTH, EAST, WEST};
        Direction candidates[4];
        int n_candidates = 0;

        // só considera direções fisicamente válidas a partir da 
        // célula atual e descarta a meia-volta
        for (int i = 0; i < 4; i++) {
            Direction d = all_dirs[i];
            if (d == opposite_dir(cur_dir)) continue;
            int nr, nc;
            if (map_valid_move(row, col, d, &nr, &nc)) {
                candidates[n_candidates++] = d;
            }
        }

        if (n_candidates == 0) {
            break;
        }

        Direction weighted[4 * 5];
        int wn = 0;
        for (int i = 0; i < n_candidates; i++) {
            int weight = (candidates[i] == cur_dir) ? 5 : 1;
            for (int w = 0; w < weight; w++) {
                weighted[wn++] = candidates[i];
            }
        }

        Direction chosen = weighted[rand() % wn];

        int nr, nc;
        map_valid_move(row, col, chosen, &nr, &nc);

        out[len][0] = nr;
        out[len][1] = nc;
        len++;

        row = nr;
        col = nc;
        cur_dir = chosen;
    }

    *out_len = len;
    return len;
}

int route_random_entry_point(int *row, int *col, Direction *dir) {
    
    for (int attempt = 0; attempt < 20; attempt++) {
        int idx = rand() % NUM_ENTRY_POINTS;
        int r = entry_points[idx].row;
        int c = entry_points[idx].col;

        Cell *cell = &sim.map[r][c];
        int free_cell;
        pthread_mutex_lock(&cell->lock);
        free_cell = (cell->occupant == -1);
        pthread_mutex_unlock(&cell->lock);

        if (free_cell) {
            *row = r;
            *col = c;
            *dir = entry_points[idx].dir;
            return 1;
        }
    }
    return 0;
}
