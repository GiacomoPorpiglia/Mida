#include "move_ordering.h"
#include <stdint.h>
#include <bits/stdc++.h>
#include "constants.h"
#include "game_constants.h"
#include "bitboard.h"
#include "board_declaration.h"

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
int history_moves[2][6][max_ply];
MOVE killer_moves[2][max_ply];
int pv_length[max_ply];
// PV table
MOVE pv_table[max_ply][max_ply];
int follow_pv = 0;
int score_pv = 0;
int count = 0;
int move_scores[256];


void enable_pv_scoring(movesList &moveList)
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

MOVE tempMove;
int tempScore, current_move, next_move, max_score, h, i, j, x, y;
void sortMoves(movesList &moveList, MOVE best_move)
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
  

    //partial inertion sort (v1.1) // improves nps by around 10%
    for(i=0; i < moveList.count; i++) {
        if(move_scores[i] > 0) {
            x = move_scores[i];
            tempMove = moveList.moves[i];
            j = i-1;
            while(j >= 0 && x > move_scores[j]) {
                move_scores[j+1] = move_scores[j];
                moveList.moves[j+1] = moveList.moves[j];
                j--;
            }
            move_scores[j+1] = x;
            moveList.moves[j+1] = tempMove;
        }
    }
}