//NNUE HEADER FILE
#include "nnue.h"

inline void init_nnue(char *filename) {
    //call NNUE lib function
    nnue_init(filename);
}

inline int evaluate_nnue(int player, int *pieces, int *squares, NNUEdata* nn) {
    return nnue_evaluate(player, pieces, squares, nn);
}

inline int evaluate_nnue_incremental(int player, int *pieces, int *squares, NNUEdata **nn_stack) {
    return nnue_evaluate_incremental(player, pieces, squares, nn_stack);
}

inline int evaluate_fen_nnue(char *fen) {
    return nnue_evaluate_fen(fen);
}