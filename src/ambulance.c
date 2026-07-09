#include <stdio.h>
#include "ambulance.h"
#include "traffic_light.h"
#include "map.h"
#include "types.h"

void *ambulance_thread(void *arg) {
    Vehicle *v = (Vehicle *)arg;

    // loop identico ao vehicle_thread, com uma diferenca:
    // antes de entrar num cruzamento, chamar light_request_priority()
    // para pedir que o semaforo vire verde na direcao correta
    //
    // fluxo por tick:
    // 1. esperar tick (pthread_cond_wait em sim.tick_cond)
    // 2. calcular proxima celula da rota
    // 3. se for CELL_INTERSECTION:
    //    - descobrir a direcao do movimento
    //    - chamar light_request_priority(light_id, direcao)
    //    - logar: printf("[AMBULANCIA %d] prioridade em (%d,%d)\n", ...)
    //    - mesmo assim aguardar o verde (a prioridade so antecipa, nao pula a fila)
    // 4. mover igual ao vehicle_thread (lock da celula, checar occupant, etc.)

    (void)v;
    return NULL;
}
