#include <time.h>
#include "clock.h"
#include "render.h"
#include "types.h"

#define SIM_DURATION_TICKS 600

void *clock_thread(void *arg) {
    (void)arg;

    struct timespec ts;
    ts.tv_sec  = TICK_MS / 1000;
    ts.tv_nsec = (TICK_MS % 1000) * 1000000L;

    while (sim.running) {
        nanosleep(&ts, NULL);

        pthread_mutex_lock(&sim.tick_lock);
        sim.tick++;
        long current_tick = sim.tick;
        pthread_cond_broadcast(&sim.tick_cond);
        pthread_mutex_unlock(&sim.tick_lock);

        render_frame();

        if (current_tick >= SIM_DURATION_TICKS) {
            pthread_mutex_lock(&sim.tick_lock);
            sim.running = 0;
            pthread_mutex_unlock(&sim.tick_lock);
        }
    }

    return NULL;
}