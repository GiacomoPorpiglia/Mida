#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include <stdint.h>
#include <bits/stdc++.h>
#include "constants.h"
#include "game_constants.h"
#include "bitboard.h"

#include "board_declaration.h"


#define WinningCaptureScore 1000000
#define firstKillerScore 900000
#define secondKillerScore 800000
#define bestMoveScore 3000000
#define pvMoveScore 2000000

#define MAX_HISTORY 16384

// most valuable victim - least valuable attacker table
/*

(credits to Maksim Korzh for this nice self-exlpainatory comment)

    (Victims) King Queen Rook   Bishop  Knight   Pawn
  (Attackers)
        King   600    500    400    300    200    100
       Queen   601    501    401    301    201    101
        Rook   602    502    402    302    202    102
      Bishop   603    503    403    303    203    103
      Knight   604    504    404    304    204    104
        Pawn   605    505    405    305    205    105

*/
// [attacker][victim]
extern const int mvv_lva[6][6];

extern MOVE killer_moves[2][max_ply];

//[color][piece type][ move target square]
extern int history_moves[2][6][max_ply];

// PV length
extern int pv_length[max_ply];

// PV table
extern MOVE pv_table[max_ply][max_ply];

// follow PV & score PV move
extern int follow_pv, score_pv; // if 1, we are following PV, if 0 we are not
extern int count;
void enable_pv_scoring(movesList *moveList);

void updateKillers(MOVE newKillerMove);
void updateHistoryScore(MOVE move, MOVE best_move, int depth, movesList* quietList, int quietMoveCount);

// score moves
/*
MOVE ORDERING
1- PV moves
2- CAPTURES
3- 1st KILLER MOVES
4- 2nd KILLER MOVES
5- HISTORY MOVES
6- UNSORTED MOVES

*/

static inline int scoreMove(MOVE move);
// extern int move_scores[256];
// sort moves to make search more efficient, by searching first captures, killers and history moves, and principle variation (PV) moves

extern MOVE tempMove;
extern int tempScore, current_move, next_move, max_score, h, i, j, x, y;
void sortMoves(movesList *moveList, MOVE best_move);
void pickNextMove(movesList *moveList, int moveNum);
#endif