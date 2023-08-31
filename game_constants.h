#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

#include <stdint.h>
// fifty move rule table
int fiftyMoveRuleTable[1000];
int plyGameCounter = 0;
int ply;

void resetFiftyMoveTable()
{
    for (int i = 0; i < 1000; i++)
        fiftyMoveRuleTable[i] = 0;
}

// repetitions table
uint64_t repetition_table[1000]; // 1000 is the number of moves possible in the entire game
int repetition_index = 0;

#endif