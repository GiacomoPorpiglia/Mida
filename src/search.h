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

extern int LMR_table[max_ply][max_ply];
extern int LMP_table[2][max_ply];
extern int LMRBase;
extern int LMRDivision;

typedef struct {
    int static_eval{};
    MOVE move{};
    MOVE excluded_move{};
    int double_extension{};
} SearchStack;

extern movesList mGen[max_ply];
extern SearchStack searchStack[max_ply + 1];

static inline bool repetitionDetection();
static inline void fillDirtyPieceNull(int d);
static inline void fillDirtyPiece(int d, MOVE move);
static inline int relativeSquare(int sq);

// returns true if the position is  draw for unsufficient material
static inline bool isInsufficientMaterial();

static inline uint64_t nonPawnMat(int side);

// quiescence search
static inline int quiescence(int alpha, int beta, SearchStack *ss);
// negamax alpha beta search
static inline int search(int depth, int alpha, int beta, SearchStack *ss);

static inline void printMove(MOVE move);

void initSearch();
void search_position(int maxDepth);

#endif