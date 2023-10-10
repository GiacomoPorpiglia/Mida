#ifndef SEARCH_POSITION_H
#define SEARCH_POSITION_H

#include <stdint.h>
#include <stdlib.h>
#include "constants.h"
#include "game_constants.h"
#include "hashing.h"
#include "board_declaration.h"
#include "evaluate.h"
#include "move_ordering.h"
#include "uci.h"

extern int nodes;
extern movesList mGen[max_ply];
// checks if the position has already occurred in the game
static inline bool repetition_detection();
static inline void fillDirtyPieceNull(int d);
static inline void fillDirtyPiece(int d, MOVE move);
static inline int relative_square(int sq);
// quiescence search
static inline int quiescence(int alpha, int beta);

extern int LMR_table[max_ply][64];
extern int LMP_table[2][8];
extern int LMRBase;
extern int LMRDivision;

typedef struct {
    int static_eval;
    MOVE move;
} SearchStack;

typedef struct {
    int searchDepth;
} Thread;

extern SearchStack searchStack[max_ply + 1];
extern Thread td;

// returns true if the position is  draw for unsufficient material
static inline bool isInsufficientMaterial();
static inline uint64_t nonPawnMat(int side);

// negamax alpha beta search
static inline int search(int depth, int alpha, int beta, SearchStack *ss);

static inline void print_move(MOVE move);

void init_search();
void search_position(int maxDepth);


#endif