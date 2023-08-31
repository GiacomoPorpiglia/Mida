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
int history_moves[2][6][max_ply] = {0};
MOVE killer_moves[2][max_ply];
int pv_length[max_ply];
// PV table
MOVE pv_table[max_ply][max_ply];
int follow_pv = 0;
int score_pv = 0;
int count = 0;


void enable_pv_scoring(movesList *moveList)
{
    // disable following PV
    follow_pv = 0;

    // loop over moves in the move list
    for (int count = 0; count < (*moveList).count; count++)
    {
        MOVE move = (*moveList).moves[count];
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

static inline int scoreMove(MOVE move)
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
    if (board.allPieces[squareTo] != NOPIECE)
        return mvv_lva[board.allPieces[squareFrom]][board.allPieces[squareTo]] + 10000;

    // special case: en passant (we consider it as pawn takes pawn)
    else if (board.allPieces[squareTo] == NOPIECE && board.allPieces[squareFrom] == P && squareTo == getEnPassantSquare(board.boardSpecs))
        return mvv_lva[P][P] + 10000;

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

void pickNextMove(movesList *moveList, int moveNum)
{

    MOVE temp;
    int index;
    int bestscore = -inf;
    int bestnum = moveNum;
    int tempScore;

    for (index = moveNum; index < (*moveList).count; index++) {
        if ((*moveList).move_scores[index] > bestscore) {
            bestscore = (*moveList).move_scores[index];
            bestnum = index;
        }
    }

    temp = (*moveList).moves[moveNum];
    tempScore = (*moveList).move_scores[moveNum];
    (*moveList).moves[moveNum] = (*moveList).moves[bestnum]; // Sort the highest score move to highest.
    (*moveList).move_scores[moveNum] = (*moveList).move_scores[bestnum];
    (*moveList).moves[bestnum] = temp;
    (*moveList).move_scores[bestnum] = tempScore;
}


void sortMoves(movesList *moveList, MOVE best_move)
{
    MOVE move;
    
    for (int count = 0; count < (*moveList).count; count++)
    {
        move = (*moveList).moves[count];
        if(best_move == move) 
            (*moveList).move_scores[count] = 30000;
        else 
            (*moveList).move_scores[count] = scoreMove(move);
    }
    
    //to delete if picknextmove available

    //partial inertion sort (v1.1) // improves nps by around 10%
    // for(i=0; i < (*moveList).count; i++) {
    //     if((*moveList).move_scores[i] > 0) {
    //         x = (*moveList).move_scores[i];
    //         tempMove = (*moveList).moves[i];
    //         j = i-1;
    //         while(j >= 0 && x > (*moveList).move_scores[j]) {
    //             (*moveList).move_scores[j+1] = (*moveList).move_scores[j];
    //             (*moveList).moves[j+1] = (*moveList).moves[j];
    //             j--;
    //         }
    //         (*moveList).move_scores[j+1] = x;
    //         (*moveList).moves[j+1] = tempMove;
    //     }
    // }
}