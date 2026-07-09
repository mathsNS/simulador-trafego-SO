#include <stdio.h>
#include "traffic_light.h"
#include "types.h"

#define NS_PHASE_TICKS 6
#define EW_PHASE_TICKS 6

static int positions[NUM_LIGHTS][2] = {
    {1, 1}, {1, 5}, {1, 9},
    {5, 1}, {5, 5}, {5, 9},
    {9, 1}, {9, 5}, {9, 9}
};

void lights_init(void) {
    for (int i = 0; i < NUM_LIGHTS; i++) {
        TrafficLight *l = &sim.lights[i];
        l->id  = i;
        l->row = positions[i][0];
        l->col = positions[i][1];

        if (i % 2 == 0) {
            l->ns_state = LIGHT_GREEN;
            l->ew_state = LIGHT_RED;
        } else {
            l->ns_state = LIGHT_RED;
            l->ew_state = LIGHT_GREEN;
        }

        l->ns_duration  = NS_PHASE_TICKS;
        l->ew_duration  = EW_PHASE_TICKS;
        l->phase_tick   = 0;
        l->ambulance_req = -1;

        pthread_mutex_init(&l->lock, NULL);
        pthread_cond_init(&l->ns_green, NULL);
        pthread_cond_init(&l->ew_green, NULL);
    }
}

static long wait_for_tick(long last_tick) {
    pthread_mutex_lock(&sim.tick_lock);
    while (sim.running && sim.tick == last_tick) {
        pthread_cond_wait(&sim.tick_cond, &sim.tick_lock);
    }
    long t = sim.tick;
    pthread_mutex_unlock(&sim.tick_lock);
    return t;
}

static int is_ns_direction(Direction dir) {
    return dir == NORTH || dir == SOUTH;
}

void *traffic_light_thread(void *arg) {
    TrafficLight *l = (TrafficLight *)arg;
    long last_tick = 0;

    while (sim.running) {
        last_tick = wait_for_tick(last_tick);
        if (!sim.running) break;

        pthread_mutex_lock(&l->lock);

        l->phase_tick++;

        int current_duration = (l->ns_state == LIGHT_GREEN) ? l->ns_duration
                                                              : l->ew_duration;

        int force_switch = 0;
        if (l->ambulance_req != -1) {
            Direction req = (Direction)l->ambulance_req;
            int req_is_ns     = is_ns_direction(req);
            int already_green = (req_is_ns  && l->ns_state == LIGHT_GREEN) ||
                                 (!req_is_ns && l->ew_state == LIGHT_GREEN);

            if (already_green) {
                l->ambulance_req = -1;
            } else {
                force_switch = 1;
            }
        }

        if (l->phase_tick >= current_duration || force_switch) {
            
            if (l->ns_state == LIGHT_GREEN) {
                l->ns_state = LIGHT_RED;
                l->ew_state = LIGHT_GREEN;
                pthread_cond_broadcast(&l->ew_green);
            } else {
                l->ns_state = LIGHT_GREEN;
                l->ew_state = LIGHT_RED;
                pthread_cond_broadcast(&l->ns_green);
            }
            l->phase_tick = 0;

            if (force_switch) {
                printf("[SEMAFORO %d] prioridade de ambulancia atendida em (%d,%d)\n",
                       l->id, l->row, l->col);
                l->ambulance_req = -1;
            }
        }

        pthread_mutex_unlock(&l->lock);
    }

    pthread_mutex_lock(&l->lock);
    pthread_cond_broadcast(&l->ns_green);
    pthread_cond_broadcast(&l->ew_green);
    pthread_mutex_unlock(&l->lock);

    return NULL;
}

void light_request_priority(int light_id, Direction dir) {
    TrafficLight *l = &sim.lights[light_id];
    pthread_mutex_lock(&l->lock);

    int dir_is_ns     = is_ns_direction(dir);
    int already_green = (dir_is_ns  && l->ns_state == LIGHT_GREEN) ||
                         (!dir_is_ns && l->ew_state == LIGHT_GREEN);

    if (!already_green) {
        
        l->ambulance_req = (int)dir;
    }
    
    pthread_mutex_unlock(&l->lock);
}