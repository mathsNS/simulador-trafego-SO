#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include "types.h"

// inicializa todos os semaforos com posicoes e estados iniciais
void lights_init(void);

// thread de cada semaforo, recebe TrafficLight* como arg
void *traffic_light_thread(void *arg);

// chamado pela ambulancia para pedir verde na direcao dir
// o semaforo deve trocar de fase assim que for seguro
void light_request_priority(int light_id, Direction dir);

#endif
