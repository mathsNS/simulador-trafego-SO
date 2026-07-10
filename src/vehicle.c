#include <stdio.h>
#include "vehicle.h"
#include "map.h"
#include "traffic_light.h"
#include "route.h"
#include "speed.h"
#include "types.h"

static int is_ns_direction(Direction dir) {
    return dir == NORTH || dir == SOUTH;
}

void vehicle_init(Vehicle *v, int id, VehicleType type,
                   int start_row, int start_col, Direction dir,
                   int route[][2], int route_len) {
    v->id = id;
    v->type = type;
    v->row = start_row;
    v->col = start_col;
    v->dir = dir;
    v->route_pos = 0;
    v->route_len = route_len;
    v->active = 1;

    v->speed_ticks = speed_ticks_for_type(type);

    for (int i = 0; i < route_len && i < MAX_ROUTE; i++) {
        v->route[i][0] = route[i][0];
        v->route[i][1] = route[i][1];
    }

    // marca a célula inicial como ocupada por este veículo
    Cell *start_cell = &sim.map[start_row][start_col];
    pthread_mutex_lock(&start_cell->lock);
    start_cell->occupant = id;
    pthread_mutex_unlock(&start_cell->lock);
}

void *vehicle_thread(void *arg) {
    Vehicle *v = (Vehicle *)arg;
    long last_tick = 0;
    int ticks_since_move = 0;

    while (1) {
        // 1. espera o próximo tick do relogio (sem busy-wait)
        pthread_mutex_lock(&sim.tick_lock);
        while (sim.running && sim.tick == last_tick) {
            pthread_cond_wait(&sim.tick_cond, &sim.tick_lock);
        }
        last_tick = sim.tick;
        int still_running = sim.running;
        pthread_mutex_unlock(&sim.tick_lock);

        if (!still_running) break;
        if (!v->active) break;

        if (v->route_pos >= v->route_len - 1) {
            v->active = 0;
            break;
        }

        // 2. só tenta se mover quando o módulo de velocidade libera
        if (!speed_should_move(&ticks_since_move, v->speed_ticks)) {
            continue;
        }
        ticks_since_move = 0;

        // 3. calcula a próxima célula da rota
        int next_row = v->route[v->route_pos + 1][0];
        int next_col = v->route[v->route_pos + 1][1];
        Direction move_dir = route_dir_between(v->row, v->col, next_row, next_col);

        int vr, vc;
        if (!map_valid_move(v->row, v->col, move_dir, &vr, &vc)) {
            // rota mal formada / movimento não permitido: encerra o veículo
            fprintf(stderr, "[VEICULO %d] movimento invalido de (%d,%d)\n",
                    v->id, v->row, v->col);
            v->active = 0;
            break;
        }
        v->dir = move_dir;

        // 4. se o destino for cruzamento, bloqueia até o sinal ficar verde
        //    na direção do movimento (sem busy-wait: pthread_cond_wait)
        Cell *next_cell = &sim.map[next_row][next_col];
        if (next_cell->type == CELL_INTERSECTION) {
            TrafficLight *light = map_get_light_at(next_row, next_col);
            if (light != NULL) {
                int is_ns = is_ns_direction(move_dir);
                pthread_mutex_lock(&light->lock);
                if (is_ns) {
                    while (sim.running && light->ns_state != LIGHT_GREEN) {
                        pthread_cond_wait(&light->ns_green, &light->lock);
                    }
                } else {
                    while (sim.running && light->ew_state != LIGHT_GREEN) {
                        pthread_cond_wait(&light->ew_green, &light->lock);
                    }
                }
                pthread_mutex_unlock(&light->lock);
            }
        }

        if (!sim.running) break;

        // 5. tenta ocupar a célula destino.
        //    estratégia anti-deadlock: nunca segura o lock de duas
        //    células ao mesmo tempo (destino e liberado antes de
        //    tocar na celula atual, e vice-versa).
        Cell *current_cell = &sim.map[v->row][v->col];

        pthread_mutex_lock(&next_cell->lock);
        if (next_cell->occupant != -1) {
            // célula ocupada: não pode avançar nem ultrapassar,
            // espera o próximo tick e tenta de novo
            pthread_mutex_unlock(&next_cell->lock);
            continue;
        }
        next_cell->occupant = v->id;
        pthread_mutex_unlock(&next_cell->lock);

        // 6. libera a célula atual e atualiza a posição do veículo
        pthread_mutex_lock(&current_cell->lock);
        current_cell->occupant = -1;
        pthread_mutex_unlock(&current_cell->lock);

        v->row = next_row;
        v->col = next_col;
        v->route_pos++;

        // 7. chegou ao fim da rota?
        if (v->route_pos >= v->route_len - 1) {
            v->active = 0;
            break;
        }
    }

    pthread_mutex_lock(&sim.map[v->row][v->col].lock);
    sim.map[v->row][v->col].occupant = -1;
    pthread_mutex_unlock(&sim.map[v->row][v->col].lock);

    return NULL;
}
