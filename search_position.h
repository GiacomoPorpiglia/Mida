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

// checks if the position has already occurred in the game
static inline int repetition_detection();

// quiescence search
static inline int quiescence(int alpha, int beta);

extern const int fullDepthMoves;
extern const int reductionLimit;

// returns true if the position is ok for applying late move reduction
static inline bool ok_to_reduce(MOVE &move);
// returns true if the position is  draw for unsufficient material
static inline bool isInsufficientMaterial();
static inline bool isEndgame();

// negamax alpha beta search
static inline int search(int depth, int alpha, int beta, bool doNull);

static inline void print_move(MOVE move);

void search_position(int maxDepth);

#endif