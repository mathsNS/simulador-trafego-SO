# Simulador de Tráfego Urbano em C

Simulação concorrente de tráfego urbano desenvolvida em **C** com **Pthreads**, para a
disciplina de Sistemas Operacionais. Veículos, ambulância, semáforos e o relógio global
são implementados como threads independentes, que disputam o acesso a recursos
compartilhados (células da malha viária, cruzamentos e semáforos) usando mutexes e
variáveis de condição - sem espera ocupada.

## Requisitos

- Linux ou WSL (usa `nanosleep` e sequências ANSI de terminal)
- `gcc`
- `make`
- `pthread` (glibc padrão em qualquer distribuição Linux)

## Compilação

```bash
make
```

Isso cria a pasta `build/` com os objetos e gera o executável `simulador` na raiz do
projeto. Para limpar os artefatos de build:

```bash
make clean
```

## Execução

```bash
./simulador
```

A simulação roda por `SIM_DURATION_TICKS` ticks (definido em `src/clock.c`) e cada tick
dura `TICK_MS` milissegundos (definido em `include/clock.h`) - ajuste esse valor para
acelerar ou desacelerar a visualização. A tela é redesenhada a cada tick; ao final, o
terminal mostra o total de ticks executados e o cursor volta a ficar visível.

## Estrutura do projeto

```
.
├── include/             # headers (.h) - contratos entre os módulos
│   ├── types.h          # structs e constantes globais (Cell, Vehicle, TrafficLight, SimState)
│   ├── map.h
│   ├── vehicle.h
│   ├── ambulance.h
│   ├── clock.h
│   ├── traffic_light.h
│   ├── route.h
│   ├── speed.h
│   └── render.h
├── src/                   # implementação (.c)
│   ├── main.c             # criação/join das threads, setup dos veículos
│   ├── map.c              # malha viária, validação de movimento
│   ├── vehicle.c          # thread de veículo comum
│   ├── ambulance.c        # thread da ambulância (prioridade em cruzamentos)
│   ├── clock.c            # relógio global (tick discreto)
│   ├── traffic_light.c    # thread de cada semáforo
│   ├── route.c            # geração de rotas aleatórias / pontos de entrada
│   ├── speed.c            # controle de velocidade por tipo de veículo
│   └── render.c           # desenho ASCII no terminal
├── build/                 # objetos gerados pelo make (não versionado)
├── Makefile
└── README.md
```

## O mapa

A malha viária é uma matriz `11x11` (`sim.map[MAP_ROWS][MAP_COLS]`, `types.h`). As ruas
horizontais ficam nas linhas 1, 5 e 9; as ruas verticais, nas colunas 1, 5 e 9,
formando ruas contínuas (não uma cruz isolada). A linha 1 é via de **mão única** (sentido
leste); as demais ruas são de **mão dupla**. As 9 interseções entre essas ruas são os
cruzamentos com semáforo (`CELL_INTERSECTION`).

Cada célula (`Cell`) guarda seu tipo, a máscara de direções permitidas, o id do veículo
ocupante e um mutex próprio, usado para proteger a ocupação da célula contra acesso
concorrente.

## Threads da simulação

| Thread | Arquivo | Quantidade | Responsabilidade |
|---|---|---|---|
| Relógio global | `clock.c` | 1 | Avança `sim.tick` a cada `TICK_MS` e acorda todas as threads via `pthread_cond_broadcast` |
| Veículo comum | `vehicle.c` | 10–19 | Move-se pela rota respeitando velocidade, mão da via e ocupação de célula |
| Ambulância | `ambulance.c` | 1 | Move-se como um veículo comum, mas solicita prioridade nos cruzamentos |
| Semáforo | `traffic_light.c` | 9 (uma por cruzamento) | Alterna as fases verde/vermelho e atende pedidos de prioridade da ambulância |

`main.c` cria todas as threads com `pthread_create` e, ao final da simulação, aguarda
todas com `pthread_join`, garantindo o encerramento limpo do programa.

## Sincronização

| Mecanismo | Onde | Para quê |
|---|---|---|
| `Cell.lock` (mutex) | uma por célula | Exclusão mútua na ocupação/liberação de células |
| `TrafficLight.lock` (mutex) | uma por cruzamento | Protege estado do semáforo e o pedido de prioridade da ambulância |
| `sim.tick_lock` (mutex) | global | Protege `sim.tick` e `sim.running` |
| `sim.tick_cond` (variável de condição) | global | Acorda veículos, ambulância e semáforos a cada tick |
| `TrafficLight.ns_green` / `ew_green` (variáveis de condição) | uma por cruzamento | Acordam veículos parados no vermelho quando a fase correspondente fica verde |

Nenhuma thread usa laços de checagem contínua (*busy waiting*): toda espera, pelo
próximo tick ou pelo sinal verde, é feita com `pthread_cond_wait` dentro de um `while`
que revalida a condição ao acordar.

**Estratégia contra deadlock:** nenhum veículo mantém o lock de duas células
simultaneamente. Ao avançar, a thread adquire e libera o lock da célula de destino
primeiro (marcando a ocupação), e só depois adquire o lock da célula de origem para
liberá-la, eliminando a condição de "hold and wait" entre locks de célula.

## Ambulância

A ambulância roda em sua própria thread (`ambulance_thread`) e segue o mesmo fluxo de
movimento dos demais veículos. Antes de entrar em um cruzamento, ela chama
`light_request_priority(light_id, direcao)`, sinalizando ao semáforo daquele cruzamento
que deve antecipar a troca de fase assim que for seguro. O semáforo registra no log
quando atende a esse pedido (`[SEMAFORO %d] prioridade de ambulancia atendida...`). A
prioridade apenas acelera a liberação do cruzamento, ela não pula a fila nem quebra a
exclusão mútua sobre as células.

## Visualização

A cada tick, `render_frame()` (`render.c`) redesenha o mapa no terminal usando
caracteres ASCII: ruas, cruzamentos, estado dos semáforos (verde/vermelho), carros e a
ambulância. Eventos relevantes (movimento inválido, pedidos e atendimentos de
prioridade da ambulância) também são registrados via `printf`/`fprintf` no terminal.

## Equipe e responsabilidades

| Integrante | Responsabilidades | 
|---|---|
| **Abner Levi** | thread do relógio (tick discreto), acordar veículos por variável de condição (sem busy-wait), lógica dos sinais (vermelho/verde, temporização), transição segura de sinal |
| **Malaquias de Oliveira** | thread da ambulância com prioridade, forçar sinal verde quando for seguro, renderização ASCII no terminal (mapa, carros, sinais, ambulância), atualização de tela a cada tick e log de eventos |
| **Maria Antônia** | thread de cada carro, geração de rota e direção, velocidade por tick (rápido/médio/lento), impenetrabilidade (mutex nas células), ausência de teletransporte e respeito à mão da via |
| **Matheus Nogueira** | structs (`Cell`, `Vehicle`, `TrafficLight`, `SimState`...), layout do mapa (matriz, ruas, cruzamentos, mão única), inicialização de mutexes/variáveis de condição, criação e join de todas as threads em `main.c`, estratégia de prevenção de deadlock |
