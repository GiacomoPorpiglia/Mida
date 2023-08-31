//NNUE HEADER FILE
#include "nnue.h"
void init_nnue(char *filename);
int evaluate_nnue(int player, int *pieces, int *squares, NNUEdata* nn);
int evaluate_nnue_incremental(int player, int *pieces, int *squares, NNUEdata **nn_stack);
int evaluate_fen_nnue(char *fen_string);