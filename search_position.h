#ifndef SEARCH_POSITION_H
#define SEARCH_POSITION_H

#include <stdint.h>
#include <stdlib.h>
#include "constants.h"
#include "game_constants.h"
#include "hashing.h"
#include "board_declaration.h"
#include "evaluate.h"
#include "move_ordering.h"
#include "uci.h"

int nodes = 0;

// checks if the position has already occurred in the game
static inline int repetition_detection()
{
    // loop over repetitions positions
    for (int i = 0; i < repetition_index; i++)
        if (repetition_table[i] == hash_key)
            return 1;

    // if no repetition found
    return 0;
}

// quiescence search
static inline int quiescence(int alpha, int beta)
{

    // every 2047 nodes
    if ((nodes & 2047) == 0)
        // "listen" to the GUI/user input
        communicate();

    nodes++;

    // draw by 50 move rule
    if (fiftyMoveRuleTable[plyGameCounter] == 100)
        return 0;

    // if we went too deep, there is overflow in killer moves, history moves and PV.
    if (ply > max_ply - 1)
        return evaluate();

    int evaluation = evaluate();
    if (evaluation >= beta)
        return beta;
    if (evaluation > alpha)
    {
        alpha = evaluation;
    }
    movesList moveList;
    if (board.colorToMove == 1)
        moveList = board.calculateWhiteMoves();
    else
        moveList = board.calculateBlackMoves();

    // sort moves
    MOVE best_move = 0;
    sortMoves(moveList, best_move);
    MOVE move;
    for (int count = 0; count < moveList.count; count++)
    {
        move = moveList.moves[count];
        int oldPieceType = board.allPieces[getSquareFrom(move)];
        int capturedPieceType = board.allPieces[getSquareTo(move)];
        // only look at captures
        if (capturedPieceType != -1)
        {
            uint16_t oldSpecs = board.boardSpecs;

            // make a copy of zobrist hash key, to restore it when unplaying the move
            uint64_t hash_key_copy = hash_key;
            playMove(move, capturedPieceType);
            ply++;
            // increment repetition index & store hash key
            repetition_index++;
            repetition_table[repetition_index] = hash_key;

            evaluation = -quiescence(-beta, -alpha);

            ply--;
            // decrement repetition index
            repetition_index--;
            hash_key = hash_key_copy;
            unplayMove(move, oldPieceType, oldSpecs, capturedPieceType);

            // return 0 if time is up
            if (stopped == 1)
                return 0;

            if (evaluation >= beta)
                return beta;
            if (evaluation > alpha)
            {
                alpha = evaluation;
            }
        }
    }
    return alpha;
}

const int fullDepthMoves = 4;
const int reductionLimit = 3;

// returns true if the position is ok for applying late move reduction
static inline bool ok_to_reduce(MOVE &move)
{

    // return true if king is not in check, move isn't a capture and move isn't a promotion
    if (!board.isInCheck && board.allPieces[getSquareTo(move)] == -1 && board.allPieces[getSquareFrom(move)] == getNewPieceType(move))
        return true;
    return false;
}

// returns true if the position is  draw for unsufficient material
static inline bool isInsufficientMaterial()
{
    return ((board.whitePiecesValue <= pieceValues[B]) && (board.blackPiecesValue <= pieceValues[B]) && !pieces_bb[WHITE][P] && !pieces_bb[BLACK][P]);
}

static inline bool isEndgame()
{
    return (board.whitePiecesValue + board.blackPiecesValue < 2000);
}

// negamax alpha beta search
static inline int search(int depth, int alpha, int beta, bool doNull)
{

    int evaluation, static_eval;

    // define hash flag
    int hash_f = HASH_FLAG_ALPHA;

    MOVE best_move = 0;

    // if repetition, return drawing score
    if (ply && repetition_detection())
        return 0;

    // draw by 50 move rule
    if (fiftyMoveRuleTable[plyGameCounter] == 100)
        return 0;

    bool pv_node = (beta - alpha) > 1;
    // read hash entry
    // 152000
    evaluation = readHashEntry(depth, alpha, beta, best_move);
    if (ply && !pv_node && (evaluation != NULL_HASH_ENTRY))
        return evaluation;

    // every 2047 nodes
    if ((nodes & 2047) == 0)
        // "listen" to the GUI/user input
        communicate();

    // init PV length
    pv_length[ply] = ply;

    if (depth == 0)
        // run quiescence search
        return quiescence(alpha, beta);

    // if we went too deep, there is overflow in killer moves, history moves and PV.
    if (ply > max_ply - 1)
        return evaluate();

    nodes++;

    movesList moveList;

    // calculate moves
    if (board.colorToMove == 1)
        moveList = board.calculateWhiteMoves();
    else
        moveList = board.calculateBlackMoves();

    if (board.isInCheck)
        depth++;

    //we calculate the static eval in this condition, because it is the same condition of reverse futility pruning and razoring, which both need the static eval
    if ((depth <= 3) && !pv_node && !board.isInCheck)
        static_eval = evaluate();

    // reverse futility pruning (v1.2) (inspired from Avalanche engine implementation)
    if ((depth <= 3) && !pv_node && !board.isInCheck) {
        int margin = depth * 200;
        if ((static_eval - margin) >= beta)
            //return beta;
            return static_eval-margin;
    }

    // null move pruning (in v1.2 a more aggressive prunign is introduced, with reduction increasing with depth)
    if (doNull && !isEndgame() && (depth >= 3) && !board.isInCheck && moveList.count && ply)
    {

        repetition_index++;
        repetition_table[repetition_index] = hash_key;

        // preserve board state, giving the opponent a free turn
        makeNullMove();
        uint16_t copySpecs = board.boardSpecs;
        unsetEnPassantSquare(board.boardSpecs);

        ply++;

        int R = depth > 6 ? 3 : 2;

        /* search moves with reduced depth to find beta cutoffs
        depth - 1 - R where R is a reduction limit */
        evaluation = -search(depth - 1 - R, -beta, -beta + 1, false);

        // restore board state
        ply--;
        repetition_index--;
        unmakeNullMove(copySpecs);
        board.boardSpecs = copySpecs;

        // reutrn 0 if time is up
        if (stopped == 1)
            return 0;

        // fail-hard beta cutoff
        if (evaluation >= beta) {
            return beta;
        }
    }

    // razoring
    if ((depth <= 3) && !pv_node && !board.isInCheck)
    {
        // add first bonus
        evaluation = static_eval + 125;
        // define new score
        int new_eval;

        // static evaluation indicates a fail-low node
        if (evaluation < beta)
        {
            // on depth 1
            if (depth == 1)
            {
                // get quiscence score
                new_eval = quiescence(alpha, beta);
                // return quiescence score if it's greater then static evaluation score

                return (new_eval > evaluation) ? new_eval : evaluation;
            }
            // add second bonus to static evaluation
            evaluation += 175;
            // static evaluation indicates a fail-low node
            if ((evaluation < beta) && (depth <= 2))
            {
                // get quiscence score
                new_eval = quiescence(alpha, beta);

                // quiescence score indicates fail-low node
                if (new_eval < beta)
                    // return quiescence score if it's greater then static evaluation score
                    return (new_eval > evaluation) ? new_eval : evaluation;
            }
        }
    }


    // if we are now followig PV line
    if (follow_pv)
        // enable PV move scoring
        enable_pv_scoring(moveList);

    // sort moves
    sortMoves(moveList, best_move);

    // number of moves searched in a move list
    int moves_searched = 0;

    if (moveList.count == 0)
    {
        if (board.isInCheck)
            return -MATE_VALUE + ply; // checkmate
        return 0;                     // stalemate
    }

    if (isInsufficientMaterial())
        return 0;

    else
    {
        MOVE move;
        for (int count = 0; count < moveList.count; count++)
        {
            move = moveList.moves[count];
            int oldPieceType = board.allPieces[getSquareFrom(move)];
            int capturedPieceType = board.allPieces[getSquareTo(move)];

            uint16_t oldSpecs = board.boardSpecs;

            bool is_ok_to_reduce = ok_to_reduce(move);

            // Copy the hash key to restore it after the search, together with unplay move
            uint64_t hash_key_copy = hash_key;
            playMove(move, capturedPieceType);
            repetition_index++;
            repetition_table[repetition_index] = hash_key;
            ply++;

            // full depth search
            if (moves_searched == 0)
                evaluation = -search(depth - 1, -beta, -alpha, doNull);
            // LMR (late move reduction)
            else
            {
                // condition to consider late move reduction (LMR)
                if (moves_searched >= fullDepthMoves && depth >= reductionLimit && is_ok_to_reduce)
                    evaluation = -search(depth - 2, -alpha - 1, -alpha, doNull); // search move with a reduced search

                else
                    // hack to ensure that full earch is done
                    evaluation = alpha + 1;

                // Principle variation search (PVS)
                if (evaluation > alpha)
                {
                    //  https://web.archive.org/web/20071030220825/http://www.brucemo.com/compchess/programming/pvs.htm
                    //  principle variation search optimization
                    /*  Once you've found a move with a score that is between alpha and beta,
                        the rest of the moves are searched with the goal of proving that they are all bad.
                        It's possible to do this a bit faster than a search that worries that one
                        of the remaining moves might be good. */
                    evaluation = -search(depth - 1, -alpha - 1, -alpha, doNull);
                    /*  If the algorithm finds out that it was wrong, and that one of the
                        subsequent moves was better than the first PV move, it has to search again,
                        in the normal alpha-beta manner.  This happens sometimes, and it's a waste of time,
                        but generally not often enough to counteract the savings gained from doing the
                        "bad move proof" search referred to earlier. */
                    if ((evaluation > alpha) && (evaluation < beta))
                        // research the move  that has failed to be proved to be bad
                        evaluation = -search(depth - 1, -beta, -alpha, doNull);
                }
            }

            ply--;
            repetition_index--;
            hash_key = hash_key_copy;
            unplayMove(move, oldPieceType, oldSpecs, capturedPieceType);

            // reutrn 0 if time is up
            if (stopped == 1)
                return 0;

            // increment number of moves searched
            moves_searched++;

            if (evaluation > alpha)
            {

                // switch hahs flag to the one storing score for PV node
                hash_f = HASH_FLAG_EXACT;

                best_move = move;

                // store history moves (only if it's a quiet move)
                if (capturedPieceType == -1)
                    history_moves[board.colorToMove][board.allPieces[getSquareFrom(move)]][getSquareTo(move)] += depth;

                alpha = evaluation;

                // write PV move
                pv_table[ply][ply] = move;

                // loop over next ply
                for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
                    // copy move from deeper ply into current ply
                    pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];

                // adjust PV length
                pv_length[ply] = pv_length[ply + 1];

                if (evaluation >= beta)
                {
                    // store hash entry
                    writeHashEntry(depth, beta, best_move, HASH_FLAG_BETA);

                    // store killer moves (only if it's a quiet move)
                    if (capturedPieceType == -1)
                    {
                        killer_moves[1][ply] = killer_moves[0][ply];
                        killer_moves[0][ply] = move;
                    }
                    return beta;
                }
            }
        }
    }

    // store hash entry
    writeHashEntry(depth, alpha, best_move, hash_f);

    return alpha;
}

static inline void print_move(MOVE move)
{
    printf("%s", coordFromPosition[getSquareFrom(move)]); // board.coordFromBitboardPosition((int)getSquareFrom(move)).c_str());
    printf("%s", coordFromPosition[getSquareTo(move)]);   // board.coordFromBitboardPosition((int)getSquareTo(move)).c_str());
    // promotion
    if ((getNewPieceType(move)) != (int)board.allPieces[(getSquareFrom(move))] && (int)board.allPieces[(getSquareFrom(move))] == 5)
    {
        if (getNewPieceType(move) == 1)
            printf("q");
        else if (getNewPieceType(move) == 2)
            printf("r");
        else if (getNewPieceType(move) == 3)
            printf("b");
        else if (getNewPieceType(move) == 4)
            printf("n");
    }
}

static inline void search_position(int maxDepth)
{
    int evaluation = 0;
    // clear helper data structures for search
    nodes = 0;
    // reset follow PV flags
    follow_pv = 0;
    score_pv = 0;
    // reset ply
    ply = 0;

    // reset "time is up" flag
    stopped = 0;

    memset(killer_moves, 0, sizeof(killer_moves));
    memset(history_moves, 0, sizeof(history_moves));
    memset(pv_table, 0, sizeof(pv_table));
    memset(pv_length, 0, sizeof(pv_length));

    int alpha = -inf;
    int beta = inf;

    // iterative deepening
    for (int curr_depth = 1; curr_depth <= maxDepth; curr_depth++)
    {
        if (stopped == 1)
        {
            // stop calculating and return best move so far
            break;
        }

        // enable follow PV flag
        follow_pv = 1;

        // search at the current depth
        evaluation = search(curr_depth, alpha, beta, true);
        // we fell outside the window, so try again with a full-width window (and the same depth)
        if ((evaluation <= alpha) || (evaluation >= beta))
        {
            alpha = -inf;
            beta = inf;
            continue;
        }

        // set up the window for the next iteration
        alpha = evaluation - 50;
        beta = evaluation + 50;

        // if PV is available
        if (pv_length[0])
        {
            // print search info
            if (evaluation > -MATE_VALUE && evaluation < -MATE_SCORE)
                printf("info score mate %d depth %d nodes %d time %d pv ", -(evaluation + MATE_VALUE) / 2 - 1, curr_depth, nodes, get_time_ms() - starttime);

            else if (evaluation > MATE_SCORE && evaluation < MATE_VALUE)
                printf("info score mate %d depth %d nodes %d time %d pv ", (MATE_VALUE - evaluation) / 2 + 1, curr_depth, nodes, get_time_ms() - starttime);

            else
                printf("info score cp %d depth %d nodes %d time %d pv ", evaluation, curr_depth, nodes, get_time_ms() - starttime);

            // loop over the moves within a PV line
            for (int count = 0; count < pv_length[0]; count++)
            {
                // print PV move
                print_move(pv_table[0][count]);
                printf(" ");
            }

            // print new line
            printf("\n");
        }
    }

    MOVE best_move = pv_table[0][0];
    printf("bestmove ");
    print_move(best_move);
    printf("\n");
}

#endif