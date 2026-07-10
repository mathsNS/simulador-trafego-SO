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

// monta uma rota horizontal de c_start ate c_end na linha row
static void make_h_route(int row, int c_start, int c_end, int out[][2], int *len) {
    int step = (c_end >= c_start) ? 1 : -1;
    int i = 0;
    for (int c = c_start; c != c_end + step; c += step) {
        out[i][0] = row;
        out[i][1] = c;
        i++;
    }
    *len = i;
}

// monta uma rota vertical de r_start ate r_end na coluna col
static void make_v_route(int col, int r_start, int r_end, int out[][2], int *len) {
    int step = (r_end >= r_start) ? 1 : -1;
    int i = 0;
    for (int r = r_start; r != r_end + step; r += step) {
        out[i][0] = r;
        out[i][1] = col;
        i++;
    }
    *len = i;
}

static void setup_vehicles(void) {
    int route[MAX_ROUTE][2];
    int len;
    int n = 0;

    // linha 1 - mao unica leste
    make_h_route(1, 1, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_FAST, 1, 1, EAST, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    make_h_route(1, 4, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_MEDIUM, 1, 4, EAST, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    // linha 5 - apenas leste (sem opostos)
    make_h_route(5, 1, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_MEDIUM, 5, 1, EAST, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    make_h_route(5, 3, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_SLOW, 5, 3, EAST, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    // linha 9 - apenas oeste (sem opostos)
    make_h_route(9, 9, 1, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_FAST, 9, 9, WEST, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    make_h_route(9, 7, 1, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_MEDIUM, 9, 7, WEST, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    // coluna 1 - apenas sul
    make_v_route(1, 2, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_SLOW, 2, 1, SOUTH, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    make_v_route(1, 4, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_FAST, 4, 1, SOUTH, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    // coluna 5 - apenas norte
    make_v_route(5, 8, 1, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_MEDIUM, 8, 5, NORTH, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    make_v_route(5, 6, 1, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_SLOW, 6, 5, NORTH, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    // ambulancia - coluna 9 sul, unico veiculo nesse trecho
    // cruza os semaforos em (5,9) e (9,9) demonstrando prioridade
    make_v_route(9, 2, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, AMBULANCE, 2, 9, SOUTH, route, len);
    pthread_create(&vehicle_threads[n], NULL, ambulance_thread, &sim.vehicles[n]); n++;

    sim.num_vehicles = n;
}

int main(void) {
    map_init();
    lights_init();
    render_init();

    setup_vehicles();

    pthread_create(&clock_tid, NULL, clock_thread, NULL);

    for (int i = 0; i < NUM_LIGHTS; i++)
        pthread_create(&light_tids[i], NULL, traffic_light_thread, &sim.lights[i]);

    pthread_join(clock_tid, NULL);

    pthread_mutex_lock(&sim.tick_lock);
    sim.running = 0;
    pthread_cond_broadcast(&sim.tick_cond);
    pthread_mutex_unlock(&sim.tick_lock);

    for (int i = 0; i < NUM_LIGHTS; i++) {
        pthread_mutex_lock(&sim.lights[i].lock);
        pthread_cond_broadcast(&sim.lights[i].ns_green);
        pthread_cond_broadcast(&sim.lights[i].ew_green);
        pthread_mutex_unlock(&sim.lights[i].lock);
    }

    for (int i = 0; i < sim.num_vehicles; i++)
        pthread_join(vehicle_threads[i], NULL);
    for (int i = 0; i < NUM_LIGHTS; i++)
        pthread_join(light_tids[i], NULL);

    printf("\nsimulacao encerrada. ticks: %ld\n", sim.tick);
    printf("\033[?25h");
    return 0;
}
