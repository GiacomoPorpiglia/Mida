#define NOMINMAX
#include "evaluate.h"
#include <stdint.h>
#include <stdlib.h>

#include "constants.h"
#include "bitboard.h"
#include "board_declaration.h"
#include "magic_bitboards.h"
#include "nnue_eval.h"
#include "nnue.h"
#include "game_constants.h"


uint64_t attacked_squares[2][7];
uint64_t double_attacked_by_pawn[2];
uint64_t double_attacked[2];


int nnue_pieces[33];
int nnue_squares[33];

static inline void nnue_input(int *pieces, int *squares) {
    uint64_t bb;
    int piece, square, idx=2;
    //white king
    bb = pieces_bb[WHITE][K];
    pieces[0] = 1;
    squares[0] = pop_lsb(bb);
    //black king
    bb = pieces_bb[BLACK][K];
    pieces[1] = 7;
    squares[1] = pop_lsb(bb);
    //white pieces
    for(int piece = Q; piece <= P; piece++) {
        bb = pieces_bb[WHITE][piece];
        while(bb) {
            square = pop_lsb(bb);
            pieces[idx]=piece+1;
            squares[idx] = square;
            idx++;
        }
    }
    for(int piece = Q; piece <= P; piece++) {
        bb = pieces_bb[BLACK][piece];
        while(bb) {
            square = pop_lsb(bb);
            pieces[idx]=piece+7;
            squares[idx] = square;
            idx++;
        }
    }

    pieces[idx] = 0;
    squares[idx] = 0;
}


NNUEdata nn;

NNUEdata nn_stack[max_ply+1];
NNUEdata* stack[3];
template<>
int evaluate<true>()
{
    nnue_input(nnue_pieces, nnue_squares);
    int side = board.colorToMove == WHITE ? BLACK : WHITE; // the NNUE side is 0 if white to move, while our WHITE is equal to 1

    //reset computed accumulation of current ply
    nn_stack[ply].accumulator.computedAccumulation = 0;
    //fill the stack with current ply and previous 2 plies' networks
    stack[0] = &nn_stack[ply];
    stack[1] = (ply > 0) ? &nn_stack[ply - 1] : 0;
    stack[2] = (ply > 1) ? &nn_stack[ply - 2] : 0;
    return evaluate_nnue_incremental(side, nnue_pieces, nnue_squares, stack);
}

template<>
int evaluate<false>() 
{
    nnue_input(nnue_pieces, nnue_squares);
    int side = board.colorToMove == WHITE ? BLACK : WHITE; // the NNUE side is 0 if white to move, while our WHITE is equal to 1

    return evaluate_nnue(side, nnue_pieces, nnue_squares, &nn_stack[ply]);
}
template int evaluate<true>();  // Explicit instantiation for true
template int evaluate<false>(); // Explicit instantiation for false

