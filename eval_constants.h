#ifndef EVAL_CONSTANTS_H
#define EVAL_CONSTANTS_H

#include <stdint.h>
#include "constants.h"

/*=========================================================\
|      arbitrary values of bonuses and penalties for       |
|     pawn structures, king safety and piece activity      |
\=========================================================*/

const int double_pawn_penalty[2] = {-15, -30};
const int isolated_pawn_penalty[2] = {-15, -30};
const int passed_pawn_bonus[8][2] = {{0, 0}, {2, 38}, {15, 36}, {22, 50}, {64, 81}, {166, 184}, {284, 269}, {0, 0}}; // for rank
const int supported_passed_pawn_by_rook_bonus[2] = {0, 35};
const int attacked_passed_pawn_by_rook_penalty[2] = {0, -35};
const int outside_passed_pawn_bonus[2] = {0, 20};
const int freeBlockSquareBonus[2] = {0, 30};

const int weakQueenProtection[2] = {7, 0};
const int hangingBonus[2] = {15, 5};
const int threatByMinor[6][2] = {
    {0, 0},
    {20, 0},
    {23, 0},
    {19, 0},
    {15, 0},
    {3, 10}
};
const int threatByRook[6][2] = {
    {0, 0},
    {20, 0},
    {0, 0},
    {15, 0},
    {14, 0},
    {2, 10}
};

const int semiopen_file_score = 15;
const int open_file_score = 20;
const int king_pawn_shield_bonus[2] = {15, 0};
const int dst_to_corner_bonus[2] = {0, 10};
const int penalty_pawns_on_same_bishop_color[2] = {3, 9};
const int bishop_on_long_diagonal_bonus[2] = {15, 0};
const int bishop_pair_bonus = 20;

const int outpostBonus[2] = {45, 20};
const int minorBehindPawnBonus[2] = {10, 3};
const int connected_pawn_bonus[2] = {2, 7};

const int bishopXRayPawns[2] = {-4, -5};
const int rookOnKingRing[2] = {13, 0};
const int bishopOnKingRing[2] = {10, 0};


const int threatByPawnPush[2] = {20, 0}; // 25 0

const uint64_t queenSide = file_masks[0] | file_masks[1] | file_masks[2] | file_masks[3];
const uint64_t kingSide = file_masks[4] | file_masks[5] | file_masks[6] | file_masks[7];

//[number of white pawns][number of black pawns]
constexpr int pawnMajorityBonus[5][5][2] = {
    {{0, 0},  {0, -20}, {0, -40}, {0, -60}, {0, -80}},
    {{0, 20}, {0, 0},   {0, -20}, {0, -40}, {0, -60}},
    {{0, 40}, {0, 20},  {0, 0},   {0, -20}, {0, -40}},
    {{0, 60}, {0, 40},  {0, 20},  {0, 0},   {0, -20}},
    {{0, 80}, {0, 60},  {0, 40},  {0, 20},  {0, 0}}
};

const int dstToCorner[64] = {
    0,
    1,
    2,
    3,
    3,
    2,
    1,
    0,
    1,
    2,
    3,
    4,
    4,
    3,
    2,
    1,
    2,
    3,
    4,
    5,
    5,
    4,
    3,
    2,
    3,
    4,
    5,
    6,
    6,
    5,
    4,
    3,
    3,
    4,
    5,
    6,
    6,
    5,
    4,
    3,
    2,
    3,
    4,
    5,
    5,
    4,
    3,
    2,
    1,
    2,
    3,
    4,
    4,
    3,
    2,
    1,
    0,
    1,
    2,
    3,
    3,
    2,
    1,
    0,
};

//stockfish
const int psqt[6][64][2] = {
    {{129, 1}, {156, 21}, {129, 41}, {94, 36}, {94, 36}, {129, 41}, {156, 21}, {129, 1}, 
    {132, 25}, {144, 48}, {111, 64}, {85, 64}, {85, 64}, {111, 64}, {144, 48}, {132, 25},
    {93, 42}, {123, 62}, {81, 81}, {57, 84}, {57, 84}, {81, 81}, {123, 62}, {93, 42},
    {78, 49}, {91, 74}, {66, 82}, {46, 82}, {46, 82}, {66, 82}, {91, 74}, {78, 49},
    {74, 46}, {85, 79}, {50, 95}, {34, 95}, {34, 95}, {50, 95}, {85, 79}, {74, 46},
    {59, 44}, {69, 82}, {39, 88}, {15, 91}, {15, 91}, {39, 88}, {69, 82}, {59, 44},
    {42, 22}, {57, 58}, {31, 55}, {16, 62}, {16, 62}, {31, 55}, {57, 58}, {42, 22}, 
    {28, 5}, {42, 28}, {21, 35}, {-1, 37}, {-1, 37}, {21, 35}, {42, 28}, {28, 5},
    },
    {{1, -33}, {-2, -27}, {-2, -22}, {2, -12}, {2, -12}, {-2, -22}, {-2, -27}, {1, -33},
    {-1, -26}, {2, -15}, {4, -11}, {6, -2}, {6, -2}, {4, -11}, {2, -15}, {-1, -26},
    {-1, -19}, {3, -9}, {6, -4}, {4, 1}, {4, 1}, {6, -4}, {3, -9}, {-1, -19},
    {2, -11}, {2, -1}, {4, 6}, {4, 11}, {4, 11}, {4, 6}, {2, -1}, {2, -11},
    {0, -14}, {6, -3}, {6, 4}, {2, 10}, {2, 10}, {6, 4}, {6, -3}, {0, -14},
    {-2, -18}, {5, -9}, {3, -5}, {4, 1}, {4, 1}, {3, -5}, {5, -9}, {-2, -18}, 
    {-2, -24}, {3, -13}, {5, -11}, {4, -4}, {4, -4}, {5, -11}, {3, -13}, {-2, -24},
    {-1, -35}, {-1, -25}, {1, -21}, {-1, -16}, {-1, -16}, {1, -21}, {-1, -25}, {-1, -35},
    },
    {{-15, -4}, {-9, -6}, {-6, -5}, {-2, -4}, {-2, -4}, {-6, -5}, {-9, -6}, {-15, -4},
    {-10, -6}, {-6, -4}, {-4, -1}, {3, -1}, {3, -1}, {-4, -1}, {-6, -4}, {-10, -6},
    {-12, 3}, {-5, -4}, {-1, -1}, {1, -3}, {1, -3}, {-1, -1}, {-5, -4}, {-12, 3},
    {-6, -3}, {-2, 1}, {-2, -4}, {-3, 4}, {-3, 4}, {-2, -4}, {-2, 1}, {-6, -3},
    {-13, -2}, {-7, 4}, {-2, 4}, {1, -3}, {1, -3}, {-2, 4}, {-7, 4}, {-13, -2}, 
    {-11, 3}, {-1, 1}, {3, -4}, {6, 5}, {6, 5}, {3, -4}, {-1, 1}, {-11, 3},
    {-1, 2}, {6, 2}, {8, 9}, {9, -2}, {9, -2}, {8, 9}, {6, 2}, {-1, 2},
    {-8, 9}, {-9, 0}, {-1, 9}, {4, 6}, {4, 6}, {-1, 9}, {-9, 0}, {-8, 9},
    },
    {{-18, -19}, {-2, -10}, {-3, -12}, {-8, -4}, {-8, -4}, {-3, -12}, {-2, -10}, {-18, -19},
    {-5, -12}, {3, -4}, {6, -6}, {1, 1}, {1, 1}, {6, -6}, {3, -4}, {-5, -12},
    {-2, -5}, {7, -1}, {-2, -1}, {6, 4}, {6, 4}, {-2, -1}, {7, -1}, {-2, -5},
    {-2, -6}, {4, -2}, {9, 0}, {13, 6}, {13, 6}, {9, 0}, {4, -2}, {-2, -6},
    {-4, -6}, {9, -1}, {7, -5}, {11, 5}, {11, 5}, {7, -5}, {9, -1}, {-4, -6}, 
    {-5, -10}, {2, 2}, {1, 1}, {4, 2}, {4, 2}, {1, 1}, {2, 2}, {-5, -10},
    {-6, -11}, {-5, -6}, {2, -1}, {0, 1}, {0, 1}, {2, -1}, {-5, -6}, {-6, -11},
    {-16, -15}, {1, -14}, {-5, -12}, {-8, -8}, {-8, -8}, {-5, -12}, {1, -14}, {-16, -15},
    },
    {{-84, -46}, {-44, -31}, {-35, -24}, {-35, -10}, {-35, -10}, {-35, -24}, {-44, -31}, {-84, -46},
    {-36, -32}, {-19, -26}, {-13, -9}, {-7, 4}, {-7, 4}, {-13, -9}, {-19, -26}, {-36, -32},
    {-29, -19}, {-8, -13}, {3, -4}, {6, 14}, {6, 14}, {3, -4}, {-8, -13}, {-29, -19},
    {-16, -16}, {4, -1}, {19, 6}, {24, 14}, {24, 14}, {19, 6}, {4, -1}, {-16, -16},
    {-16, -21}, {6, -8}, {21, 4}, {24, 19}, {24, 19}, {21, 4}, {6, -8}, {-16, -21}, 
    {-4, -24}, {11, -21}, {28, -8}, {25, 8}, {25, 8}, {28, -8}, {11, -21}, {-4, -24},
    {-32, -33}, {-13, -24}, {2, -24}, {18, 6}, {18, 6}, {2, -24}, {-13, -24}, {-32, -33},
    {-96, -48}, {-39, -42}, {-26, -26}, {-12, -8}, {-12, -8}, {-26, -26}, {-39, -42}, {-96, -48},
    },
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {1, -4}, {2, -3}, {5, 4}, {9, 2}, {8, 8}, {10, 3}, {4, -3}, {-1, -9},
    {-4, -4}, {-7, -4}, {5, -5}, {7, 2}, {12, 1}, {11, 1}, {3, -4}, {-9, -2}, 
    {-1, 4}, {-9, 1}, {4, -4}, {9, -1}, {19, -6}, {8, -6}, {1, -5}, {-2, -3},
    {5, 6}, {-2, 3}, {-5, 1}, {1, -3}, {5, -2}, {0, -2}, {-6, 6}, {2, 4},
    {1, 13}, {-5, 9}, {-3, 9}, {11, 14}, {-4, 14}, {-2, 4}, {-6, 4}, {-5, 6},
    {-4, -1}, {3, -6}, {-1, 6}, {-5, 11}, {2, 11}, {-6, 8}, {5, 4}, {-4, 4},
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    }
};

// bonus to each piece according to how many squares they can go to
constexpr int mobilityBonus[][32][2] = {
    {}, // King

    {{-14, -24}, {-8, -14}, {-4, -4}, {-4, 8}, {9, 19}, {12, 27}, {11, 29}, {18, 36}, {20, 38}, {27, 47}, {32, 47}, {34, 50}, {34, 62}, {35, 64}, {35, 66}, {35, 66}, {35, 68}, {36, 70}, {37, 73}, {38, 74}, {45, 76}, {52, 84}, {52, 85}, {53, 85}, {56, 89}, {57, 92}, {57, 93}, {59, 110}}, // Queen

    {{-30, -41}, {-12, -7}, {0, 8}, {1, 21}, {2, 36}, {7, 50}, {10, 51}, {15, 61}, {20, 66}, {20, 69}, {20, 76}, {22, 80}, {28, 82}, {29, 85}, {33, 87}}, // Rook

    {{-23, -29}, {-10, -12}, {7, -4}, {14, 6}, {19, 10}, {26, 20}, {26, 28}, {30, 29}, {31, 32}, {34, 36}, {39, 39}, {41, 43}, {45, 44}, {48, 49}}, // Bishop

    {{-31, -39}, {-26, -28}, {-6, -15}, {-1, -8}, {1, 3}, {6, 6}, {10, 8}, {14, 10}, {18, 13}} // Knight
};

// penalty based onhow many squares the king sees (meaning that it is more exposed). To calculate the squarees seen by the king, the king is seen as a queen, therefore seeing in all 8 directions
const int virtualMobilityPenalty[28][2] = {{0, 0}, {0, 0}, {-5, 0}, {-8, 0}, {-18, 0}, {-25, 0}, {-29, 0}, {-37, 0}, {-41, 0}, {-54, 0}, {-65, 0}, {-68, 0}, {-69, 0}, {-70, 0}, {-70, 0}, {-70, 0}, {-71, 0}, {-72, 0}, {-74, 0}, {-76, 0}, {-90, 0}, {-104, 0}, {-105, 0}, {-106, 0}, {-112, 0}, {-114, 0}, {-114, 0}, {-119, 0}};

//weight of the attacker indexed by piece type 
const int attackerWeight[6] = {
    0, 80, 40, 20, 20, 0
};
//attackWeight indexed by number of attackers
const int attackWeight[8] = {
    0,
    0,
    50,
    75,
    88,
    94,
    97,
    99
};
#endif