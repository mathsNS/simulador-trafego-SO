#include <stdio.h>
#include <pthread.h>

#include "ambulance.h"
#include "traffic_light.h"
#include "map.h"
#include "types.h"
#include "route.h"
#include "speed.h"

extern SimState sim;

// Função auxiliar para determinar se a direção é Norte ou Sul 
static int is_ns_direction(Direction dir) {
    return dir == NORTH || dir == SOUTH;
}

void *ambulance_thread(void *arg)
{
    Vehicle *v = (Vehicle *)arg;
    long last_tick = 0;
    int ticks_since_move = 0;

    while (sim.running && v->active)
    {
        //ESPERAR TICK (pthread_cond_wait em sim.tick_cond)
          
        pthread_mutex_lock(&sim.tick_lock);
        while (sim.running && sim.tick == last_tick) {
            pthread_cond_wait(&sim.tick_cond, &sim.tick_lock);
        }
        last_tick = sim.tick;
        int still_running = sim.running;
        pthread_mutex_unlock(&sim.tick_lock);

        // Se a simulação parou ou a ambulância desativou, encerra a thread
        if (!still_running || !v->active) {
            break;
        }

        // Se já atingiu o fim da rota, encerra
        if (v->route_pos >= v->route_len - 1) {
            v->active = 0;
            break;
        }

        // Controla a velocidade (ambulância usa speed_ticks correspondente)
        if (!speed_should_move(&ticks_since_move, v->speed_ticks)) {
            continue;
        }
        ticks_since_move = 0;

        //CALCULAR PRÓXIMA CÉLULA DA ROTA
    
        int next_row = v->route[v->route_pos + 1][0];
        int next_col = v->route[v->route_pos + 1][1];
        Direction move_dir = route_dir_between(v->row, v->col, next_row, next_col);

        int vr, vc;
        if (!map_valid_move(v->row, v->col, move_dir, &vr, &vc)) {
            fprintf(stderr, "[AMBULANCIA %d] movimento invalido de (%d,%d)\n", v->id, v->row, v->col);
            v->active = 0;
            break;
        }
        v->dir = move_dir;

        //SE FOR CELL_INTERSECTION: PEDIR PRIORIDADE E AGUARDAR
          
        Cell *next_cell = &sim.map[next_row][next_col];
        if (next_cell->type == CELL_INTERSECTION)
        {
            TrafficLight *light = map_get_light_at(next_row, next_col);
            if (light != NULL)
            {
                // - Chamar light_request_priority para pedir verde
                light_request_priority(light->id, move_dir);
                
                // - Logar a solicitação de prioridade 
                printf("[AMBULANCIA %d] prioridade em (%d,%d)\n", v->id, next_row, next_col);

                // aguardar o verde (a prioridade só antecipa, não pula a fila)
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

        if (!sim.running) {
            break;
        }

        //MOVER IGUAL AO vehicle_thread (Anti-deadlock)
        Cell *current_cell = &sim.map[v->row][v->col];

        // Tenta ocupar a célula de destino
        pthread_mutex_lock(&next_cell->lock);
        if (next_cell->occupant != -1) {
            // Célula ocupada: solta o lock e tenta de novo no próximo tick
            pthread_mutex_unlock(&next_cell->lock);
            continue;
        }
        next_cell->occupant = v->id;
        pthread_mutex_unlock(&next_cell->lock);

        // Libera a célula que a ambulância estava ocupando anteriormente
        pthread_mutex_lock(&current_cell->lock);
        current_cell->occupant = -1;
        pthread_mutex_unlock(&current_cell->lock);

        // Atualiza as variáveis de estado da ambulância
        v->row = next_row;
        v->col = next_col;
        v->route_pos++;

        // Se chegou ao fim da rota pós-movimento, encerra de forma limpa
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