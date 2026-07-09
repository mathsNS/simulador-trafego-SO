#include <stdio.h>
#include <pthread.h>
#include "types.h"
#include "map.h"
#include "vehicle.h"
#include "clock.h"
#include "traffic_light.h"
#include "ambulance.h"
#include "render.h"

SimState sim;

static pthread_t vehicle_threads[MAX_VEHICLES];
static pthread_t clock_tid;
static pthread_t light_tids[NUM_LIGHTS];

static void setup_vehicles(void) {
    // TODO: definir rotas e criar os veiculos com vehicle_init
    // TODO: pthread_create para cada carro (vehicle_thread) e para a ambulancia (ambulance_thread)
    // pelo menos 10 carros + 1 ambulancia
}

int main(void) {
    // TODO: map_init()
    // TODO: lights_init()
    // TODO: render_init()
    // TODO: setup_vehicles()
    // TODO: pthread_create para clock_tid e cada light_tids[i]
    // TODO: pthread_join no clock (ele encerra a sim)
    // TODO: setar sim.running = 0 e broadcast em tick_cond + ns_green + ew_green
    //       pra desbloquear todas as threads presas em cond_wait
    // TODO: pthread_join em todos os veiculos e semaforos
    return 0;
}
