#ifndef VEHICLE_H
#define VEHICLE_H

#include "types.h"

// preenche a struct Vehicle e marca a celula inicial como ocupada
void vehicle_init(Vehicle *v, int id, VehicleType type,
                  int start_row, int start_col, Direction dir,
                  int route[][2], int route_len);

// funcao da thread de cada carro comum
void *vehicle_thread(void *arg);

#endif
