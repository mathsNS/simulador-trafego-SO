#ifndef TYPES_H
#define TYPES_H

#include <pthread.h>

#define MAP_ROWS        11
#define MAP_COLS        11
#define MAX_VEHICLES    20
#define MAX_ROUTE       128
#define NUM_LIGHTS      9

// flags de direcao para bitmask
#define DIR_N 0x1
#define DIR_S 0x2
#define DIR_E 0x4
#define DIR_W 0x8

typedef enum {
    NORTH = 0,
    SOUTH,
    EAST,
    WEST
} Direction;

typedef enum {
    CAR_FAST,       // move a cada 1 tick
    CAR_MEDIUM,     // move a cada 2 ticks
    CAR_SLOW,       // move a cada 4 ticks
    AMBULANCE
} VehicleType;

typedef enum {
    CELL_WALL,
    CELL_ROAD,
    CELL_INTERSECTION
} CellType;

typedef enum {
    LIGHT_GREEN,
    LIGHT_RED
} LightState;

// uma celula do mapa
typedef struct {
    CellType type;
    int      allowed;    // bitmask de DIR_* com as direcoes permitidas
    int      occupant;   // id do veiculo na celula, -1 se livre
    pthread_mutex_t lock;
} Cell;

// semaforo de um cruzamento
// ns controla quem vai em N/S, ew controla quem vai em E/W
typedef struct {
    int id;
    int row, col;

    LightState ns_state;
    LightState ew_state;
    int ns_duration;     // ticks de cada fase
    int ew_duration;
    int phase_tick;      // ticks decorridos na fase atual

    int ambulance_req;   // direcao solicitada pela ambulancia (-1 = nenhuma)

    pthread_mutex_t  lock;
    pthread_cond_t   ns_green;  // sinalizado quando N/S fica verde
    pthread_cond_t   ew_green;  // sinalizado quando E/W fica verde
} TrafficLight;

// um veiculo/thread
typedef struct {
    int         id;
    VehicleType type;
    int         row, col;
    Direction   dir;
    int         speed_ticks;        // ticks entre movimentos
    int         route[MAX_ROUTE][2]; // sequencia de (row, col) a percorrer
    int         route_len;
    int         route_pos;
    int         active;              // 0 = terminou
} Vehicle;

// estado global da simulacao
typedef struct {
    Cell         map[MAP_ROWS][MAP_COLS];
    TrafficLight lights[NUM_LIGHTS];
    Vehicle      vehicles[MAX_VEHICLES];
    int          num_vehicles;

    long tick;
    int  running;

    pthread_mutex_t tick_lock;
    pthread_cond_t  tick_cond;   // clock faz broadcast a cada tick

    pthread_mutex_t render_lock; // impede que o render leia estado a meio update
} SimState;

extern SimState sim;

#endif
