#ifndef RENDER_H
#define RENDER_H

#include "types.h"

// esconde o cursor e prepara o terminal
void render_init(void);

// redesenha o mapa completo no terminal
// chamado pelo clock a cada tick
void render_frame(void);

#endif
