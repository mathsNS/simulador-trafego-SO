#include <time.h>
#include "clock.h"
#include "render.h"
#include "types.h"

#define SIM_DURATION_TICKS 600

void *clock_thread(void *arg) {
    (void)arg;

    // TODO: a cada TICK_MS milissegundos (usar nanosleep ou usleep):
    // 1. incrementar sim.tick
    // 2. fazer broadcast em sim.tick_cond pra acordar todos os veiculos e semaforos
    //    usar pthread_mutex_lock/unlock em sim.tick_lock ao redor do broadcast
    // 3. chamar render_frame()
    // 4. quando sim.tick >= SIM_DURATION_TICKS, setar sim.running = 0 e sair

    return NULL;
}
