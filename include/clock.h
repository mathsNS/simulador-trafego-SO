#ifndef CLOCK_H
#define CLOCK_H

#include "types.h"

#define TICK_MS 400   // duracao de um tick em ms, ajustar pra mais rapido/lento

// thread do relogio global
// a cada TICK_MS faz broadcast em sim.tick_cond
void *clock_thread(void *arg);

#endif
