#include <stdio.h>
#include "traffic_light.h"
#include "types.h"

#define NS_PHASE_TICKS 6
#define EW_PHASE_TICKS 6

// posicoes dos cruzamentos no mapa (mesma ordem do mapa em map.c)
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

        // alterna fase inicial pra nao ficarem todos no mesmo estado
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

void *traffic_light_thread(void *arg) {
    TrafficLight *l = (TrafficLight *)arg;

    while (sim.running) {
        // TODO: esperar o proximo tick usando sim.tick_cond (igual ao vehicle_thread)
        // TODO: incrementar l->phase_tick dentro de l->lock
        // TODO: quando phase_tick atingir a duracao da fase, trocar ns_state e ew_state
        // TODO: ao virar verde, fazer broadcast no cond var correto (ns_green ou ew_green)
        //       pra acordar os carros que estavam bloqueados nesse semaforo
        // TODO: se ambulance_req != -1, antecipar a troca de fase pra essa direcao
        //       e zerar ambulance_req depois
        // IMPORTANTE: a troca de estado deve acontecer com l->lock travado
        //             assim nenhum carro entra no cruzamento durante a mudanca
    }

    // acorda qualquer carro preso aqui quando a sim terminar
    pthread_mutex_lock(&l->lock);
    pthread_cond_broadcast(&l->ns_green);
    pthread_cond_broadcast(&l->ew_green);
    pthread_mutex_unlock(&l->lock);
    return NULL;
}

void light_request_priority(int light_id, Direction dir) {
    TrafficLight *l = &sim.lights[light_id];
    pthread_mutex_lock(&l->lock);
    l->ambulance_req = (int)dir;
    // TODO: se a direcao pedida ja for verde, nao precisa fazer nada
    // TODO: senao, forcar uma troca de fase antecipada
    pthread_mutex_unlock(&l->lock);
}
