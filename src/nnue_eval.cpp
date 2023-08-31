#include "nnue_eval.h"
#include "nnue.h"

void init_nnue(char *filename) {
    //call NNUE lib function
    nnue_init(filename);
}

int evaluate_nnue(int player, int *pieces, int *squares, NNUEdata* nn) {
    return nnue_evaluate(player, pieces, squares, nn);
}

int evaluate_nnue_incremental(int player, int *pieces, int *squares, NNUEdata **nn_stack) {
    return nnue_evaluate_incremental(player, pieces, squares, nn_stack);
}

int evaluate_fen_nnue(char *fen) {
    return nnue_evaluate_fen(fen);
}