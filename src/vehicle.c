#include <stdio.h>
#include "vehicle.h"
#include "map.h"
#include "traffic_light.h"
#include "types.h"

void vehicle_init(Vehicle *v, int id, VehicleType type,
                  int start_row, int start_col, Direction dir,
                  int route[][2], int route_len) {
    // TODO: preencher todos os campos da struct Vehicle
    // nao esquecer de definir speed_ticks conforme o tipo (1, 2 ou 4)
    // marcar a celula inicial como ocupada (occupant = id) usando o lock da celula
    (void)v; (void)id; (void)type;
    (void)start_row; (void)start_col; (void)dir;
    (void)route; (void)route_len;
}

void *vehicle_thread(void *arg) {
    Vehicle *v = (Vehicle *)arg;

    // loop principal: roda enquanto a sim estiver ativa e o veiculo tiver rota
    //
    // a cada iteracao:
    // 1. bloquear em sim.tick_cond esperando o proximo tick (sem busy-wait)
    //    usar pthread_cond_wait dentro de pthread_mutex_lock/unlock em sim.tick_lock
    // 2. verificar se e a vez deste veiculo mover (baseado em speed_ticks)
    // 3. calcular a proxima celula da rota
    // 4. se for CELL_INTERSECTION: verificar o semaforo
    //    bloquear em ns_green ou ew_green conforme a direcao de movimento
    //    isso evita busy-wait no sinal vermelho
    // 5. adquirir o lock da celula destino e checar se occupant == -1
    //    se estiver ocupada, pular o tick (nao pode ultrapassar)
    // 6. mover: liberar celula atual, ocupar celula destino, atualizar v->row/col
    // 7. avancar route_pos; se chegou ao fim, setar v->active = 0
    //
    // IMPORTANTE: a estrategia contra deadlock e adquirir sempre um lock por vez
    // nunca segurar o lock de uma celula enquanto espera o lock de outra

    (void)v;
    return NULL;
}
