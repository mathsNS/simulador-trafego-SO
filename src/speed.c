#include "speed.h"
#include "types.h"

int speed_ticks_for_type(VehicleType type) {
    switch (type) {
        case CAR_FAST:   return 1;
        case CAR_MEDIUM: return 2;
        case CAR_SLOW:   return 4;
        case AMBULANCE:  return 1;
        default:         return 1;
    }
}

int speed_should_move(int *ticks_since_move, int speed_ticks) {
    // protege contra configuração inválida (0 ou negativo) para nunca
    // travar o veículo tentando dividir/comparar errado
    if (speed_ticks < 1) {
        speed_ticks = 1;
    }

    (*ticks_since_move)++;

    if (*ticks_since_move < speed_ticks) {
        return 0;
    }

    *ticks_since_move = 0;
    return 1;
}
