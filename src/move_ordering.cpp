#include "move_ordering.h"
#include <stdint.h>
#include <bits/stdc++.h>
#include "see.h"

const int mvv_lva[6][6] = {
    600,500,400,300,200,100,
    601,501,401,301,201,101,
    602,502,402,302,202,102,
    603,503,403,303,203,103,
    604,504,404,304,204,104,
    605,505,405,305,205,105,
};
int16_t history_moves[2][6][64] = {0};
MOVE killer_moves[2][max_ply];
int pv_length[max_ply];

// PV table
MOVE pv_table[max_ply][max_ply];
int follow_pv = 0;
int score_pv = 0;
int count = 0;


void enable_pv_scoring(movesList *moveList) {
    // disable following PV
    follow_pv = 0;

    // loop over moves in the move list
    for (int count = 0; count < moveList->count; count++) {

        MOVE move = moveList->moves[count];
        // if we hit PV move
        if (move == pv_table[0][ply]) {
            score_pv = 1;
            // enable  following PV
            follow_pv = 1;
            break;
        }
    }
}


void updateKillers(MOVE newKillerMove) {
    killer_moves[1][ply] = killer_moves[0][ply];
    killer_moves[0][ply] = newKillerMove;
}

void updateHistoryScore(MOVE best_move, int depth, movesList* quietList, SearchStack* ss) {

    int bonus = std::min(2100, 300 * depth -300);
    
    if (depth > 2) {

        int pieceType = board.allPieces[getSquareFrom(best_move)];
        int squareTo  = getSquareTo(best_move);

        //update history
        int16_t *history = &history_moves[board.colorToMove][pieceType][squareTo];
        *history += bonus - ((*history) * std::abs(bonus) / MAX_HISTORY);

        //update continuation history
        int16_t *cont_history;
        if((ss - 1)->move) {
            cont_history = &((ss - 1)->continuation_history[board.colorToMove][pieceType][squareTo]);
            *cont_history += bonus - ((*cont_history) * std::abs(bonus) / MAX_HISTORY);
        }
        if((ss - 2)->move) {
            cont_history = &((ss - 2)->continuation_history[board.colorToMove][pieceType][squareTo]);
            *cont_history += bonus - ((*cont_history) * std::abs(bonus) / MAX_HISTORY);
        }
        if ((ss - 4)->move) {
            cont_history = &((ss - 4)->continuation_history[board.colorToMove][pieceType][squareTo]);
            *cont_history += bonus - ((*cont_history) * std::abs(bonus) / MAX_HISTORY);
        }
        if ((ss - 6)->move) {
            cont_history = &((ss - 6)->continuation_history[board.colorToMove][pieceType][squareTo]);
            *cont_history += bonus - ((*cont_history) * std::abs(bonus) / MAX_HISTORY);
        }
    }

    for (int i = 0; i < quietList->count; i++) {
        
        MOVE quiet_move = quietList->moves[i];
        if (quiet_move == best_move)
            continue;

        int pieceType = board.allPieces[getSquareFrom(quiet_move)];
        int squareTo = getSquareTo(quiet_move);
        // penalize history of moves which didn't cause beta-cutoffs
        int16_t *history = &history_moves[board.colorToMove][pieceType][squareTo];
        
        *history += -bonus - ((*history) * std::abs(bonus) / MAX_HISTORY);

        int16_t *cont_history;

        // update continuation history

        if((ss - 1)->move) {
            cont_history = &((ss - 1)->continuation_history[board.colorToMove][pieceType][squareTo]);
            *cont_history += -bonus - ((*cont_history) * std::abs(bonus) / MAX_HISTORY);
        }
        if((ss - 2)->move) {
            cont_history = &((ss - 2)->continuation_history[board.colorToMove][pieceType][squareTo]);
            *cont_history += -bonus - ((*cont_history) * std::abs(bonus) / MAX_HISTORY);
        }
        
        if((ss - 4)->move) {
            cont_history = &((ss - 4)->continuation_history[board.colorToMove][pieceType][squareTo]);
            *cont_history += -bonus - ((*cont_history) * std::abs(bonus) / MAX_HISTORY);
        }
        if((ss - 6)->move) {
            cont_history = &((ss - 6)->continuation_history[board.colorToMove][pieceType][squareTo]);
            *cont_history += -bonus - ((*cont_history) * std::abs(bonus) / MAX_HISTORY);
        }
    }
}

int get_history(MOVE move, SearchStack* ss, int side, int ply) {
    int pieceType = board.allPieces[getSquareFrom(move)];
    int to        = getSquareTo(move);
    int history = (int) history_moves[side][pieceType][to];
    if (ply >= 1 && (ss - 1)->move)
        history += (int) (ss - 1)->continuation_history[side][pieceType][to];
    if (ply >= 2 && (ss - 2)->move)
        history += (int)(ss - 2)->continuation_history[side][pieceType][to];
    if (ply >= 4 && (ss - 4)->move)
        history += (int)(ss - 4)->continuation_history[side][pieceType][to];
    if (ply >= 6 && (ss - 6)->move)
        history += (int)(ss - 6)->continuation_history[side][pieceType][to];

    return history;
 
}

static inline int scoreMove(MOVE move, SearchStack* ss) {

    // if PV move scoring is allowed and if we are scoring PV move
    if (score_pv && pv_table[0][ply] == move) {
        
        // disable score PV flag
        score_pv = 0;
        // score PV move giving it the highest score to search it first
        return pvMoveScore;

    }

    int squareFrom   = getSquareFrom(move);
    int squareTo     = getSquareTo(move);
    int pieceType    = board.allPieces[squareFrom];
    int newPieceType = getNewPieceType(move);

    int score = 0;

    // check if it's a capture
    if (isCapture(move))
        score += mvv_lva[pieceType][board.allPieces[squareTo]] + WinningCaptureScore * see(move, -105);

    // special case: en passant (we consider it as pawn takes pawn)
    else if (isEnPassant(move))
        score += mvv_lva[P][P];

    //if is capture, score it as a good SEE capture + MVV_LVA
    else if (isPromotion(move)) {
        score += mvv_lva[P][newPieceType] + WinningCaptureScore * (newPieceType==Q);
    }

    // score quiet move
    else {
        // score 1st killer move
        if (killer_moves[0][ply] == move)
            score += firstKillerScore;
        // score 2nd killer move
        else if (killer_moves[1][ply] == move)
            score += secondKillerScore;
        else {
            // else, score history move
            score += history_moves[board.colorToMove][pieceType][squareTo];

            if((ss - 1)->move) {
                score += (ss - 1)->continuation_history[board.colorToMove][pieceType][squareTo];
            }
            if((ss - 2)->move) {
                score += (ss - 2)->continuation_history[board.colorToMove][pieceType][squareTo];
            }
            if ((ss - 4)->move) {
                score += (ss - 4)->continuation_history[board.colorToMove][pieceType][squareTo];
            }
            if ((ss - 6)->move) {
                score += (ss - 6)->continuation_history[board.colorToMove][pieceType][squareTo];
            }
        }
    }

    return score;
}

MOVE tempMove;
int tempScore, current_move, next_move, max_score, h, i, j, x, y;

void pickNextMove(movesList *moveList, int moveNum) {
    MOVE temp;
    int index;
    int bestscore = -MAX_MOVE_SCORE;
    int bestnum = moveNum;
    int tempScore;

    for (index = moveNum; index < moveList->count; index++) {
        if (moveList->move_scores[index] > bestscore) {
            bestscore = moveList->move_scores[index];
            bestnum   = index;
        }
    }

    temp                           = moveList->moves[moveNum];
    tempScore                      = moveList->move_scores[moveNum];
    moveList->moves[moveNum]       = moveList->moves[bestnum]; // Sort the highest score move to highest.
    moveList->move_scores[moveNum] = moveList->move_scores[bestnum];
    moveList->moves[bestnum]       = temp;
    moveList->move_scores[bestnum] = tempScore;
}

/* give a score to every move, which will then be used to decide what moves to search first
   a previous implementation sorted all the moves after scoring them, while now they are picked
   one by one ( by the pickNextMove function ) whenever we need a new move to search: this approach is faster the more you cut, because
   you will need to pick a small number of moves compared to all the moves of the position
*/
void scoreMoves(movesList *moveList, MOVE best_move, SearchStack* ss) {
    MOVE move;
    board.getTotalAttackedSquares(board.get_occupancy());
    for (int count = 0; count < moveList->count; count++) {
        move = moveList->moves[count];
        if(best_move == move) 
            moveList->move_scores[count] = bestMoveScore;
        else 
            moveList->move_scores[count] = scoreMove(move, ss);
    }
}