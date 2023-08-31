#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

#include <stdint.h>
// fifty move rule table
extern int fiftyMoveRuleTable[1000];
extern int plyGameCounter; // = 0;
extern int ply;

void resetFiftyMoveTable();
// repetitions table
extern uint64_t repetition_table[1000]; // 1000 is the number of moves possible in the entire game
extern int repetition_index; // = 0;

#endif