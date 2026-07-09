#ifndef AMBULANCE_H
#define AMBULANCE_H

#include "types.h"

// thread da ambulancia, recebe Vehicle* como arg
// funciona igual a vehicle_thread mas pede prioridade nos cruzamentos
void *ambulance_thread(void *arg);

#endif
