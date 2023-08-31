#include "search.h"
#include <stdint.h>
#include <stdlib.h>
#include "constants.h"
#include "game_constants.h"
#include "hashing.h"
#include "board_declaration.h"
#include "evaluate.h"
#include "move_ordering.h"
#include "bitboard.h"
#include "uci.h"
#include <cmath>

int nodes = 0;

int LMR_table[max_ply][64];
int LMRBase = 75;
int LMRDivision = 300;

void init_search() {
    //Init LMR table
    float base = LMRBase / 100.0f;
    float division = LMRDivision / 100.0f;
    for(int depth = 1; depth < max_ply; depth++) {
        for(int played = 1; played < 64; played++) {
            LMR_table[depth][played] = base + log(depth) * log(played) / division; // formula from RICE engine
        }
    }
}

static inline int relative_square(int sq) {
    if(board.colorToMove) return sq;
    return FLIP(sq);
}

//populate the dirty piece for current ply in case of null move(from CFish)
static inline void fillDirtyPieceNull(int ply) {
    DirtyPiece *dp = &(nn_stack[ply].dirtyPiece);
    dp->dirtyNum=0;
    dp->pc[0]=0;
}

// populate the dirty piece for current ply (from CFish)
static inline void fillDirtyPiece(int ply, MOVE move) {
    int rfrom, rto;
    int from = getSquareFrom(move);
    int to   = getSquareTo(move);
    
    int oldPieceType = board.allPieces[getSquareFrom(move)];
    int capturedPieceType = board.allPieces[getSquareTo(move)];
    int newPieceType = getNewPieceType(move);

    DirtyPiece* dp = &(nn_stack[ply].dirtyPiece);

    // (*dp)={};

    dp->dirtyNum=1;
    
    //castle
    if(oldPieceType==K && abs(from-to)==2) {

        int kingSide = to > from;
        rfrom = relative_square(kingSide ? 7 : 0);
        rto   = relative_square(kingSide ? 5 : 3);

        dp->dirtyNum = 2;
        dp->pc[1]    = board.colorToMove ? pieces::wrook : pieces::brook;
        dp->from[1]  = rfrom;
        dp->to[1]    = rto;
    }
    //capture
    else if(capturedPieceType!=NOPIECE) {
        dp->dirtyNum = 2;
        dp->pc[1]    = board.colorToMove==WHITE ? (capturedPieceType+7) : (capturedPieceType+1);
        dp->from[1]  = to;
        dp->to[1]    = 64;
    }

    else if (oldPieceType == P && to==getEnPassantSquare(board.boardSpecs)) {
        dp->dirtyNum = 2;
        dp->pc[1]    = board.colorToMove==WHITE ?  pieces::bpawn : pieces::wpawn;
        dp->from[1]  = board.colorToMove==WHITE ? (to-8) : (to+8);
        dp->to[1]    = 64;
    }
    dp->pc[0]   = board.colorToMove==WHITE ?  (oldPieceType + 1) : (oldPieceType+7);
    dp->from[0] = from;
    dp->to[0]   = to;
    // promotion
    if (oldPieceType != newPieceType && oldPieceType==P && !(to > 7 && to < 56))
    {
        dp->to[0]             = 64; // pawn to SQ_NONE, promoted piece from SQ_NONE
        dp->pc[dp->dirtyNum]  = board.colorToMove==WHITE ? (newPieceType + 1) : (newPieceType+7);
        dp->from[dp->dirtyNum] = 64;
        dp->to[dp->dirtyNum]   = to;
        dp->dirtyNum++;
    }
}



static inline bool repetition_detection()
{
    // loop over repetitions positions
    for (int i = 0; i < repetition_index; i++)
        if (repetition_table[i] == hash_key)
            return true;

    // if no repetition found
    return false;
}

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
        return evaluate<false>();

    int evaluation = evaluate<true>();
    
    //Delta pruning
    if(evaluation < alpha-pieceValues[Q]) return alpha;

    if (evaluation > alpha)
    {
        if (evaluation >= beta)
            return beta;
        alpha = evaluation;
    }
    movesList moveList;
    if (board.colorToMove == 1)
        moveList = board.calculateWhiteMoves();
    else
        moveList = board.calculateBlackMoves();

    // sort moves
    sortMoves(&moveList, 0);
    MOVE move;
    for (int count = 0; count < moveList.count; count++)
    {   
        pickNextMove(&moveList, count);
        move = moveList.moves[count];
        int oldPieceType = board.allPieces[getSquareFrom(move)];
        int capturedPieceType = board.allPieces[getSquareTo(move)];
        // only look at captures and promotions
        if (capturedPieceType != NOPIECE)
        {
            uint16_t oldSpecs = board.boardSpecs;

            // make a copy of zobrist hash key, to restore it when unplaying the move
            uint64_t hash_key_copy = hash_key;

            fillDirtyPiece(ply+1, move);
            
            playMove(move);
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

static inline bool ok_to_reduce(MOVE move)
{

    // return true if king is not in check, move isn't a capture and move isn't a promotion
    return (!board.isInCheck && !isCapture(move) && !isPromotion(move));
}

static inline bool isInsufficientMaterial()
{
    return ((board.whitePiecesValue <= pieceValues[B]) && (board.blackPiecesValue <= pieceValues[B]) && !pieces_bb[WHITE][P] && !pieces_bb[BLACK][P]);
}

static inline uint64_t nonPawnMat(int side)
{
    return (pieces_bb[side][Q] | pieces_bb[side][R] | pieces_bb[side][B] | pieces_bb[side][N]);
}


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
    bool is_root = (ply==0);
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

    if (depth <= 0)
        // run quiescence search
        return quiescence(alpha, beta);

    // if we went too deep, there is overflow in killer moves, history moves and PV.
    if (ply > max_ply - 1)
        return evaluate<false>();

    nodes++;

    movesList moveList;

    // calculate moves
    if (board.colorToMove == WHITE)
        moveList = board.calculateWhiteMoves();
    else
        moveList = board.calculateBlackMoves();

    if (board.isInCheck)
        depth++;

    if (moveList.count == 0) {
        if (board.isInCheck)
            return -MATE_VALUE + ply; // checkmate
        return 0;                     // stalemate
    }

    //we calculate the static eval in this condition, because it is the same condition of reverse futility pruning and razoring, which both need the static eval
    static_eval = evaluate<true>();


    if(!pv_node && !board.isInCheck && !is_root) {

        //reverse futility pruning
        if(depth < 9  && (static_eval - depth*80)>= beta)
            return static_eval;

        //null move pruning
        if(doNull && nonPawnMat(board.colorToMove) && (depth >= 3) ) {
            repetition_index++;
            repetition_table[repetition_index] = hash_key;

            // preserve board state, giving the opponent a free turn

            fillDirtyPieceNull(ply + 1);
            makeNullMove();
            uint16_t copySpecs = board.boardSpecs;
            unsetEnPassantSquare(board.boardSpecs);

            ply++;

            int R = 2 + depth / 3; // depth > 6 ? 3 : 2;  could prove useful in the future;

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
            if (evaluation >= beta)
            {
                return evaluation;
            }
        }

        //razoring
        if(depth <= 3) {
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
    }

    // if we are now followig PV line
    if (follow_pv)
        // enable PV move scoring
        enable_pv_scoring(&moveList);

    // sort moves
    sortMoves(&moveList, best_move);

    if (isInsufficientMaterial())
        return 0;

    else
    {
        MOVE move;
        for (int moveCount = 0; moveCount < moveList.count; moveCount++)
        {
            pickNextMove(&moveList, moveCount);
            move = moveList.moves[moveCount];
            int oldPieceType = board.allPieces[getSquareFrom(move)];
            int capturedPieceType = board.allPieces[getSquareTo(move)];
            uint16_t oldSpecs = board.boardSpecs;
            bool is_ok_to_reduce = ok_to_reduce(move);

            // Copy the hash key to restore it after the search, together with unplay move
            uint64_t hash_key_copy = hash_key;
            fillDirtyPiece(ply+1, move);
            
            playMove(move);
            repetition_index++;
            repetition_table[repetition_index] = hash_key;
            ply++;
            // full depth search
            if (moveCount == 0)
                evaluation = -search(depth - 1, -beta, -alpha, true);
            // LMR (late move reduction)
            else
            {
                // condition to consider late move reduction (LMR)
                if ((moveCount >= 4) && (depth >= 3) && is_ok_to_reduce)
                {
                    int R = LMR_table[std::min(depth, 63)][std::min(moveCount, 63)]; //  (moveCount < 10) ? 1 : depth / 3;       // reduce the first 6 moves by 1, and the other moves (more unlikely than the previous ones) by depth / 3
                    evaluation = -search(depth - 1 - R, -alpha - 1, -alpha, true); // search move with a reduced search
                }

                else
                    // hack to ensure that full search is done
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
                    evaluation = -search(depth - 1, -alpha - 1, -alpha, true);
                    /*  If the algorithm finds out that it was wrong, and that one of the
                        subsequent moves was better than the first PV move, it has to search again,
                        in the normal alpha-beta manner.  This happens sometimes, and it's a waste of time,
                        but generally not often enough to counteract the savings gained from doing the
                        "bad move proof" search referred to earlier. */
                    if ((evaluation > alpha) && (evaluation < beta))
                        // research the move  that has failed to be proved to be bad
                        evaluation = -search(depth - 1, -beta, -alpha, true);
                }
            }

            ply--;
            repetition_index--;
            hash_key = hash_key_copy;
            unplayMove(move, oldPieceType, oldSpecs, capturedPieceType);

            // reutrn 0 if time is up
            if (stopped == 1)
                return 0;


            if (evaluation > alpha)
            {

                // switch hahs flag to the one storing score for PV node
                hash_f = HASH_FLAG_EXACT;

                best_move = move;

                // store history moves (only if it's a quiet move)
                if (capturedPieceType == NOPIECE)
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
                    if (capturedPieceType == NOPIECE)
                    {
                        killer_moves[1][ply] = killer_moves[0][ply];
                        killer_moves[0][ply] = move;
                    }
                    return evaluation;
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
        if (getNewPieceType(move) == Q)
            printf("q");
        else if (getNewPieceType(move) == R)
            printf("r");
        else if (getNewPieceType(move) == B)
            printf("b");
        else if (getNewPieceType(move) == N)
            printf("n");
    }
}

void search_position(int maxDepth)
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

    int delta = 50;

    // iterative deepening
    for (int curr_depth = 1; curr_depth <= maxDepth; curr_depth++)
    {
        ply = 0;
        for(int i = 0; i < 65; i++) {
            nn_stack[i].accumulator.computedAccumulation=0;
        }

        if (stopped == 1)
        {
            // stop calculating and return best move so far
            break;
        }

        // enable follow PV flag
        follow_pv = 1;

        alpha = evaluation - delta;
        beta  = evaluation + delta;
        if(curr_depth <= 3) {
            alpha = -inf;
            beta  =  inf;
        }

        // search at the current depth
        evaluation = search(curr_depth, alpha, beta, true);
        //we fell outside the aspiration window, so try again with a full-width window (and the same depth)
        if(evaluation <= alpha) {
            alpha = evaluation-delta;
            delta += delta/2;
            curr_depth--;
            continue;
        }
        if(evaluation >= beta) {
            beta = evaluation+delta;
            delta += delta/2;
            curr_depth--;
            continue;
        }

        delta = 50;
        

        // if ((evaluation <= alpha) || (evaluation >= beta))
        // {
        //     alpha = -inf;
        //     beta = inf;
        //     curr_depth--;
        //     continue;
        // }

        
        // // set up the window for the next iteration
        // alpha = evaluation - 50;
        // beta  = evaluation + 50;

        // if PV is available
        if (pv_length[0])
        {
            int nps = (float)(nodes*1000)/(get_time_ms() - starttime);
            nps = nps > 0 ? nps : 0;
            // print search info
            if (evaluation > -MATE_VALUE && evaluation < -MATE_SCORE)
                printf("info score mate %d depth %d nodes %d nps %d time %d pv ", -(evaluation + MATE_VALUE) / 2 - 1, curr_depth, nodes, nps, get_time_ms() - starttime);

            else if (evaluation > MATE_SCORE && evaluation < MATE_VALUE)
                printf("info score mate %d depth %d nodes %d nps %d time %d pv ", (MATE_VALUE - evaluation) / 2 + 1, curr_depth, nodes, nps, get_time_ms() - starttime);

            else
                printf("info score cp %d depth %d nodes %d nps %d time %d pv ", evaluation, curr_depth, nodes, nps, get_time_ms() - starttime);

            
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