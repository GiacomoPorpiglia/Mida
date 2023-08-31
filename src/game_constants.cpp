#include "game_constants.h"

int plyGameCounter = 0;
int repetition_index = 0;
uint64_t repetition_table[1000];
int fiftyMoveRuleTable[1000];
int ply;
void resetFiftyMoveTable()
{
    for (int i = 0; i < 1000; i++)
        fiftyMoveRuleTable[i] = 0;
}
