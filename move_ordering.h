#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include <stdint.h>
#include <bits/stdc++.h>
#include "constants.h"
#include "game_constants.h"
#include "bb_helpers.h"

#include "board_declaration.h"

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
const int mvv_lva[6][6] = {
    600,
    500,
    400,
    300,
    200,
    100,
    601,
    501,
    401,
    301,
    201,
    101,
    602,
    502,
    402,
    302,
    202,
    102,
    603,
    503,
    403,
    303,
    203,
    103,
    604,
    504,
    404,
    304,
    204,
    104,
    605,
    505,
    405,
    305,
    205,
    105,
};

MOVE killer_moves[2][max_ply];

//[color][piece type][ move target square]
int history_moves[2][6][max_ply];

// PV length
int pv_length[max_ply];

// PV table
MOVE pv_table[max_ply][max_ply];

// follow PV & score PV move
int follow_pv, score_pv; // if 1, we are following PV, if 0 we are not
int count;
static inline void enable_pv_scoring(movesList &moveList)
{
    // disable following PV
    follow_pv = 0;

    // loop over moves in the move list
    for (int count = 0; count < moveList.count; count++)
    {
        MOVE move = moveList.moves[count];
        // if we hit PV move
        if (move == pv_table[0][ply])
        {
            score_pv = 1;
            // enable  following PV
            follow_pv = 1;
            break;
        }
    }
}

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

static inline int scoreMove(MOVE &move)
{
    // if PV move scoring is allowed
    if (score_pv)
    {
        // if we are scoring PV move
        if (pv_table[0][ply] == move)
        {
            // disable score PV flag
            score_pv = 0;
            // score PV move giving it the highest score to search it first
            return 20000;
        }
    }

    int squareFrom = getSquareFrom(move);
    int squareTo = getSquareTo(move);
    // check if it's a capture
    if (board.allPieces[squareTo] != -1)
        return mvv_lva[board.allPieces[squareFrom]][board.allPieces[squareTo]] + 10000;

    // special case: en passant (we consider it as pawn takes pawn)
    else if (board.allPieces[squareTo] == -1 && board.allPieces[squareFrom] == 5 && squareTo == getEnPassantSquare(board.boardSpecs))
        return mvv_lva[5][5] + 10000;

    // score quiet move
    else
    {
        // score 1st killer move
        if (killer_moves[0][ply] == move)
            return 9000;
        // score 2nd killer move
        else if (killer_moves[1][ply] == move)
            return 8000;
        // score history move
        else
            return history_moves[board.colorToMove][board.allPieces[squareFrom]][squareTo];
    }
    return 0;
}

int move_scores[256];
// sort moves to make search more efficient, by searching first captures, killers and history moves, and principle variation (PV) moves

MOVE tempMove;
int tempScore, current_move, next_move;
static inline void sortMoves(movesList &moveList, MOVE best_move)
{
    MOVE move;
    
    for (int count = 0; count < moveList.count; count++)
    {
        move = moveList.moves[count];
        if(best_move == move) 
            move_scores[count] = 30000;
        else 
            move_scores[count] = scoreMove(move);
    }
    
    for (current_move = 0; current_move < moveList.count-1; current_move++)
    {
        for (next_move = current_move + 1; next_move < moveList.count; next_move++)
        {
            if (move_scores[current_move] < move_scores[next_move])
            {
                tempMove = moveList.moves[current_move];
                tempScore = move_scores[current_move];
                // swap scores
                move_scores[current_move] = move_scores[next_move];
                move_scores[next_move] = tempScore;
                // swap moves
                moveList.moves[current_move] = moveList.moves[next_move];
                moveList.moves[next_move] = tempMove;
            }
        }
    }
}

#endif