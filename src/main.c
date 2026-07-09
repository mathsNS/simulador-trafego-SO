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

    // linha 1 mao unica leste
    make_h_route(1, 1, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_FAST, 1, 1, EAST, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    // linha 5 leste e oeste (sentidos opostos, vao se cruzar)
    make_h_route(5, 1, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_MEDIUM, 5, 1, EAST, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    make_h_route(5, 9, 1, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_SLOW, 5, 9, WEST, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    // linha 9 leste e oeste
    make_h_route(9, 1, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_FAST, 9, 1, EAST, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    make_h_route(9, 9, 1, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_MEDIUM, 9, 9, WEST, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    // coluna 1 sul e norte
    make_v_route(1, 2, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_SLOW, 2, 1, SOUTH, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    make_v_route(1, 8, 1, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_FAST, 8, 1, NORTH, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    // coluna 5 sul e norte
    make_v_route(5, 2, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_MEDIUM, 2, 5, SOUTH, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    make_v_route(5, 8, 1, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_SLOW, 8, 5, NORTH, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    // coluna 9 sul e norte
    make_v_route(9, 2, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_FAST, 2, 9, SOUTH, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    make_v_route(9, 8, 1, route, &len);
    vehicle_init(&sim.vehicles[n], n, CAR_MEDIUM, 8, 9, NORTH, route, len);
    pthread_create(&vehicle_threads[n], NULL, vehicle_thread, &sim.vehicles[n]); n++;

    // ambulancia linha 9 cruzando os semaforos em (9,5) e (9,9)
    make_h_route(9, 3, 9, route, &len);
    vehicle_init(&sim.vehicles[n], n, AMBULANCE, 9, 3, EAST, route, len);
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
