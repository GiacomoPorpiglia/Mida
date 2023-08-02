#ifndef ENGINE_CONSTANTS_H
#define ENGINE_CONSTANTS_H
#define CH char
#define MOVE uint16_t
#define ILLEGAL_MOVE 0
// transposition table hash flags
#define HASH_FLAG_EXACT 0
#define HASH_FLAG_ALPHA 1
#define HASH_FLAG_BETA 2

// bounds for the range of mating scores
// [-infinity, -mate_value ... -mate_score, normal_scores, ...mate_score, ...mate value, infinity]
#define MATE_SCORE 48000
#define MATE_VALUE 49000

#define HASH_TABLE_SIZE 8000000 // 200 MB
#define NULL_HASH_ENTRY 100000  // to make sure it goes outside the alpha-beta window

#define WHITE 1
#define BLACK 0


extern const int pieceValues[6];

const int K = 0;
const int Q = 1;
const int R = 2;
const int B = 3;
const int N = 4;
const int P = 5;
const int ALL_PIECES = 6;
// extern const int K;
// extern const int Q;
// extern const int R;
// extern const int B;
// extern const int N;
// extern const int P;
// extern const int ALL_PIECES;

#define inf 50000

#define max_ply 64

#define WIN32_LEAN_AND_MEAN
#define startpos "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <stdint.h>
#include <bit>

typedef struct
{
    MOVE moves[256];
    int count;
} movesList;

// not a constant. it is here because it is needed everywhere, and this file is imported everywhere.
//
//  [side][piece type (also all pieces, index = 6)]
extern uint64_t pieces_bb[2][7];

/*
knight attack table, with a bitboard for each square
the knight can be in, precomputed for faster performance
*/
extern uint64_t knight_attacks[64]; 

/*
king attack table, with a bitboard for each square
the knight can be in, precomputed for faster performance
*/
extern uint64_t king_attacks[64];
/*
white pawn attack table, with a bitboard for each square
the pawn can be in, precomputed for faster performance
*/
extern uint64_t white_pawn_attacks[64];

/*
black pawn attack table, with a bitboard for each square
the pawn can be in, precomputed for faster performance
*/
extern uint64_t black_pawn_attacks[64];

// file masks for each square, where the hot bits of the mask represent the highlighted file
extern const uint64_t file_masks[64];

// rank masks for each square, where the hot bits of the mask represent the highlighted rank
extern const uint64_t rank_masks[64];
// rank indices for each position
extern const uint64_t get_rank[64];

extern const char coordFromPosition[64][3];
// isolated pawn masks for each position (basically if a pawn is on the F file, the isolated mask will be the E file + G file, meaning the neighboring files)
extern const uint64_t isolated_masks[64];

// white passed pawn mask, it shows all the squares in front of a pawn on its file and the 2 adjacent files.
extern uint64_t white_passed_mask[64];

// black passed pawn mask, it shows all the squares in front of a pawn on its file and the 2 adjacent files.
extern uint64_t black_passed_mask[64];

extern const uint64_t black_squares_bb;
extern const uint64_t white_squares_bb;

extern const uint64_t queenSide;
extern const uint64_t kingSide;
extern const uint64_t CenterFiles;

//indexed by [<file>]
extern const uint64_t kingFlank[8];

extern const uint64_t center;

#endif