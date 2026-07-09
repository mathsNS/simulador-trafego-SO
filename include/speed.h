#ifndef SPEED_H
#define SPEED_H

#include "types.h"

// número de ticks que um veículo espera entre dois movimentos consecutivos
int speed_ticks_for_type(VehicleType type);

// decide, a cada tick recebido do relógio global, se o veículo pode
// tentar se mover nesse tick
int speed_should_move(int *ticks_since_move, int speed_ticks);

#endif
