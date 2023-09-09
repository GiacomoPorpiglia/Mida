#ifndef EVALUATE_H
#define EVALUATE_H

#include <stdint.h>
#include "constants.h"
#include "bitboard.h"
#include "nnue.h"

template <bool useIncrementalNNUE> int evaluate();
extern NNUEdata nn;
extern NNUEdata nn_stack[max_ply+1];

extern int nnue_pieces[33];
extern int nnue_squares[33];


static inline void nnue_input(int *pieces, int *squares);

/*=========================================================\
|      arbitrary values of bonuses and penalties for       |
|     pawn structures, king safety and piece activity      |
\=========================================================*/
extern uint64_t attacked_squares[2][7];
extern uint64_t double_attacked_by_pawn[2];
extern uint64_t double_attacked[2];

#endif