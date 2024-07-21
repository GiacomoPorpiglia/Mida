#ifndef TYPES_H
#define TYPES_H
#include "constants.h"

typedef struct {
    MOVE moves[MAX_MOVES];
    int move_scores[MAX_MOVES];
    int count;
} movesList;

typedef struct {
    int static_eval{};
    MOVE move{};
    MOVE prevMove{};

    int16_t continuation_history[2][6][64];

} SearchStack;

#endif