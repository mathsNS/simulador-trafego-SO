#ifndef ROUTE_H
#define ROUTE_H

#include "types.h"

// número de pontos de entrada conhecidos na malha
#define NUM_ENTRY_POINTS 11

// retorna a direção de movimento entre duas células adjacentes.
// usada tanto pelo gerador de rota quanto pelas threads de veiculo
Direction route_dir_between(int row, int col, int next_row, int next_col);

// gera uma rota aleatória válida começando em (start_row start_col)
int route_generate_random(int start_row, int start_col, Direction start_dir,
                           int min_len, int out[][2], int *out_len);

// sorteia um ponto de entrada válido na malha junto com a direção correta de entrada
int route_random_entry_point(int *row, int *col, Direction *dir);

#endif
