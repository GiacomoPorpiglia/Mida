#include "search.h"
#include <stdint.h>
#include <stdlib.h>
#include "bitboard.h"
#include "constants.h"
#include <cmath>
#include "see.h"

// Score of v2.2 vs v2.1: 118 - 16 - 66 [0.755]
// ...      v2.2 playing White: 70 - 8 - 23  [0.807] 101
// ...      v2.2 playing Black: 48 - 8 - 43  [0.702] 99
// ...      White vs Black: 78 - 56 - 66  [0.555] 200
// Elo difference: 195.5 +/- 42.0, LOS: 100.0 %, DrawRatio: 33.0 %
// 200 of 200 games finished.

int nodes = 0;

movesList mGen[max_ply];

SearchStack searchStack[max_ply + 1];

int LMR_table[max_ply][64];
int LMP_table[2][64];
int LMRBase = 30;
int LMRDivision = 230;

void initSearch() {
    //Init LMR table
    float base = LMRBase / 100.0f;
    float division = LMRDivision / 100.0f;
    for(int depth = 1; depth < max_ply; depth++) {
        for(int played = 1; played < 64; played++) {
            LMR_table[depth][played] = base + log(depth) * log(played) / division; // formula from Berserk engine
        }
    }

    LMR_table[0][0] = LMR_table[1][0] =  LMR_table[0][1] = 0;

    for(int depth = 1; depth < 64; depth++) {
        LMP_table[0][depth] = 1.5 +  0.4 * depth * depth;
        LMP_table[1][depth] = 2.5 +  0.9 * depth * depth;
    }
}

static inline int relativeSquare(int sq) {
    return board.colorToMove ? sq : FLIP(sq);
}

//populate the dirty piece for current ply in case of null move(from CFish)
static inline void fillDirtyPieceNull(int ply) {
    DirtyPiece *dp = &(nn_stack[ply].dirtyPiece);
    dp->dirtyNum = 0;
    dp->pc[0]    = 0;
}

// populate the dirty piece for current ply (from CFish NNUE implementation)
static inline void fillDirtyPiece(int ply, MOVE move) {
    int rfrom, rto;
    int from = getSquareFrom(move);
    int to   = getSquareTo(move);
    
    int oldPieceType      = board.allPieces[getSquareFrom(move)];
    int capturedPieceType = board.allPieces[getSquareTo(move)];
    int newPieceType      = getNewPieceType(move);

    DirtyPiece* dp = &(nn_stack[ply].dirtyPiece);

    dp->dirtyNum = 1;
    
    //castle
    if(isCastle(move)) {

        int kingSide = to > from;
        rfrom = relativeSquare(kingSide ? 7 : 0);
        rto   = relativeSquare(kingSide ? 5 : 3);

        dp->dirtyNum = 2;
        dp->pc[1]    = board.colorToMove ? pieces::wrook : pieces::brook;
        dp->from[1]  = rfrom;
        dp->to[1]    = rto;
    }

    //capture
    else if(isCapture(move)) {
        dp->dirtyNum = 2;
        dp->pc[1]    = board.colorToMove==WHITE ? (capturedPieceType+7) : (capturedPieceType+1);
        dp->from[1]  = to;
        dp->to[1]    = 64;
    }

    else if (isEnPassant(move)) {
        dp->dirtyNum = 2;
        dp->pc[1]    = board.colorToMove==WHITE ?  pieces::bpawn : pieces::wpawn;
        dp->from[1]  = board.colorToMove==WHITE ? (to-8) : (to+8);
        dp->to[1]    = 64;
    }
    dp->pc[0]   = board.colorToMove==WHITE ?  (oldPieceType + 1) : (oldPieceType+7);
    dp->from[0] = from;
    dp->to[0]   = to;

    // promotion
    if (isPromotion(move)) {
        dp->to[0]              = 64; // pawn to SQ_NONE, promoted piece from SQ_NONE
        dp->pc[dp->dirtyNum]   = board.colorToMove==WHITE ? (newPieceType + 1) : (newPieceType+7);
        dp->from[dp->dirtyNum] = 64;
        dp->to[dp->dirtyNum]   = to;
        dp->dirtyNum++;
    }
}



static inline bool repetitionDetection() {
    // loop over repetitions positions
    for (int i = 0; i < repetition_index; i++)
        if (repetition_table[i] == hash_key)
            return true;

    // if no repetition found
    return false;
}

static inline int quiescence(int alpha, int beta, SearchStack *ss) {
    int best_score = -MATE_VALUE;
    int standing_pat, evaluation;
    MOVE best_move;
    // every 2047 nodes
    if ((nodes & 2047) == 0)
        // "listen" to the GUI/user input
        communicate();

    // draw by 50 move rule
    if (fiftyMoveRuleTable[plyGameCounter] == 100)
        return 0;

    // if we went too deep, simply return the evaluation (stop the search)
    if (ply > max_ply - 1)
        return evaluate<false>();
    
    bool pv_node = (beta - alpha) > 1;

    standing_pat = evaluate<true>();

    ss->static_eval = standing_pat;

    //Delta pruning
    if(standing_pat < alpha-pieceValues[Q]) return alpha;

    if (standing_pat > alpha) {
        if (standing_pat >= beta)
            return beta;
        alpha = standing_pat;
    }
    
    tt* ttEntry = readHashEntry(best_move);

    if (ttEntry!=nullptr && !pv_node) {
        evaluation = ttEntry->eval;
        if ((ttEntry->flag == HASH_FLAG_EXACT ||
            (ttEntry->flag == HASH_FLAG_ALPHA && ttEntry->eval <= alpha) ||
            (ttEntry->flag == HASH_FLAG_BETA  && ttEntry->eval >= beta)))
            return evaluation;
    }


    movesList *moveList = &mGen[ply];

    board.calculateMoves(board.colorToMove, moveList);

    // sort moves
    scoreMoves(moveList, 0);
    MOVE move;
    int playedCount = 0; //counter of played moves

    best_score = standing_pat;
    best_move = NULL_MOVE; 

    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        pickNextMove(moveList, moveCount);
        move = moveList->moves[moveCount];
        
        // only look at captures and promotions
        if (isCapture(move) || isEnPassant(move) || isPromotion(move)) {
            /*
            In quiescence, we can skip captures evaluated as losing by SEE with confidence that they will not result in a better position
            */
            if (moveList->move_scores[moveCount] < WinningCaptureScore && playedCount >= 1)
                continue;
            
            int oldPieceType      = board.allPieces[getSquareFrom(move)];
            int capturedPieceType = board.allPieces[getSquareTo(move)];

            uint16_t oldSpecs = board.boardSpecs;

            // make a copy of zobrist hash key, to restore it when unplaying the move
            uint64_t hash_key_copy = hash_key;

            fillDirtyPiece(ply+1, move);
            playedCount++;
            playMove(move);
            ply++;

            nodes++;

            ss->move = move;
            // increment repetition index & store hash key
            repetition_index++;
            repetition_table[repetition_index] = hash_key;

            evaluation = -quiescence(-beta, -alpha, ss + 1);

            ply--;
            
            repetition_index--;

            hash_key = hash_key_copy;
            unplayMove(move, oldPieceType, oldSpecs, capturedPieceType);

            // return 0 if time is up
            if (stopped)
                return 0;

            //we beat the best move
            if(evaluation > best_score) {
                best_score = evaluation;
                best_move  = move;
                if(evaluation > alpha) {
                    alpha = evaluation;
                    if(evaluation >= beta)
                        break;
                    
                }
            }
        }
    }
    int flag = best_score >= beta ? HASH_FLAG_BETA : HASH_FLAG_ALPHA;
    writeHashEntry(0, best_score, best_move, flag);
    return best_score;
}


static inline bool isInsufficientMaterial() {
    return ((board.whitePiecesValue <= pieceValues[B]) && (board.blackPiecesValue <= pieceValues[B]) && !pieces_bb[WHITE][P] && !pieces_bb[BLACK][P]);
}

static inline uint64_t nonPawnMat(int side) {
    return (pieces_bb[side][Q] | pieces_bb[side][R] | pieces_bb[side][B] | pieces_bb[side][N]);
}


static inline int search(int depth, int alpha, int beta, SearchStack* ss) {

    int evaluation, static_eval=0;

    // define hash flag
    int hash_f = HASH_FLAG_ALPHA;

    MOVE best_move = 0;

    // if repetition, return drawing score
    if (ply && repetitionDetection())
        return 0;

    // draw by 50 move rule
    if (fiftyMoveRuleTable[plyGameCounter] == 100)
        return 0;

    bool pv_node = (beta - alpha) > 1;
    bool is_root = (ply==0);


    // read hash entry
    tt* ttEntry = readHashEntry(best_move);
    bool ttHit = (ttEntry != nullptr);

    MOVE excluded_move = ss->excluded_move;

    if(excluded_move != NULL_MOVE) ttHit = false;

    if (ttHit && ply && !pv_node) {
        ttHit = true;
        static_eval = ttEntry->eval;
        if (ttEntry->depth >= depth &&
            (ttEntry->flag == HASH_FLAG_EXACT ||
            (ttEntry->flag == HASH_FLAG_ALPHA && ttEntry->eval <= alpha) ||
            (ttEntry->flag == HASH_FLAG_BETA  && ttEntry->eval >= beta)))
            return static_eval;
    }

    // every 2047 nodes
    if ((nodes & 2047) == 0)
        // "listen" to the GUI/user input
        communicate();

    // init PV length
    pv_length[ply] = ply;

    if (depth <= 0)
        // run quiescence search
        return quiescence(alpha, beta, ss);

    // if we went too deep, there is overflow in killer moves, history moves and PV.
    if (ply > max_ply - 1)
        return evaluate<false>();

    bool in_check = board.inCheck();

    if(in_check)
        depth++;

    /*
    if we have the static_eval from the tt entry, 
    we can use it because it is more accurate 
    than the evaluation of the position from the NN,
    because it comes from a search (even if at shallow depth). 
    The reason why I still add it the NN eval and do a weighted avg is 
    because I haven't found a way to make the NN work without evaluating every single position. 
    So, I still have to evaluate the position even if I could use only the tt entry eval. :(
    */
    static_eval = static_eval ? (static_eval*4+evaluate<true>())/5 : evaluate<true>();

    ss->static_eval = static_eval;

    bool improving = ply >= 2 && !in_check && (ss->static_eval > (ss-2)->static_eval);

    (ss + 1)->excluded_move = NULL_MOVE;
    (ss)->double_extension  = !is_root ? (ss - 1)->double_extension : 0; 

    movesList *moveList = &mGen[ply];
    bool are_moves_calculated = false;

    if(!pv_node && !in_check && !is_root && !excluded_move) {

        //reverse futility pruning
        
        if(depth < 9  && (static_eval - depth*80) >= beta)
            return static_eval;

        //null move pruning

        if((ss-1)->move != NULL_MOVE && nonPawnMat(board.colorToMove) && (depth >= 3) && static_eval >= beta) {
            repetition_index++;
            repetition_table[repetition_index] = hash_key;

            // preserve board state, giving the opponent a free turn

            fillDirtyPieceNull(ply + 1);
            makeNullMove();
            uint16_t copySpecs = board.boardSpecs;
            unsetEnPassantSquare(board.boardSpecs);

            ss->move = NULL_MOVE;

            ply++;

            /*
            We increase the reduction with depth, and also
            depending on how far we are from beta. If we are far from beta,
            chances are we won't find a beta cutoff and so we can use a bigger
            reduction factor.
            */
            int R = 3 + depth / 3 + std::min(3, (static_eval-beta) / 180);

            //adjust reduction to not exceed the depth
            R = std::min(depth, R);

            /* 
            search moves with reduced depth to find beta cutoffs
            */
            evaluation = -search(depth - R, -beta, -beta + 1, ss + 1);

            // restore board state
            ply--;
            repetition_index--;
            unmakeNullMove(copySpecs);
            board.boardSpecs = copySpecs;

            // reutrn 0 if time is up
            if (stopped)
                return 0;

            // fail-hard beta cutoff
            if (evaluation >= beta) {
                //if we find a mate, we can't return it because it might not be true. We return beta instead
                if(evaluation >= MATE_SCORE) evaluation = beta;
                return evaluation;
            }
            
        }

        //razoring

        if(depth <= 2 && static_eval + 180*depth <= alpha) {
            return quiescence(alpha, beta, ss);
        }
    }


    /*
    mate distance pruning

    if a mate has been found, this pruning helps to discard 
    all branches that can't reach mate in less plies 
    than the current mate found.
    */
    int matingValue = MATE_VALUE - ply;
    if (matingValue < beta) {
        beta = matingValue;
        if (alpha >= matingValue) return matingValue; // Beta cutoff
    }

    // calculate moves
    if(!are_moves_calculated) {
        board.calculateMoves(board.colorToMove, moveList);
    }
    
    //if no moves are availble, it's either checkmate or stalemate
    if (moveList->count == 0) {
        if (in_check)
            return -MATE_VALUE + ply; // checkmate
        return 0;                     // stalemate
    }

    // if we are now followig PV line
    if (follow_pv)
        // enable PV move scoring
        enable_pv_scoring(moveList);

    // sort moves
    scoreMoves(moveList, best_move);

    if (isInsufficientMaterial())
        return 0;

    else {

        MOVE move;
        int  quietMoveCount=0;
        bool skip_quiet_moves = false;

        movesList quietList;

        for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
            pickNextMove(moveList, moveCount);
            move = moveList->moves[moveCount];

            if (move == ss->excluded_move)
                continue;

            bool isKillerMove = (moveList->move_scores[moveCount] == firstKillerScore) || (moveList->move_scores[moveCount] == secondKillerScore);

            int oldPieceType      = board.allPieces[getSquareFrom(move)];
            int capturedPieceType = board.allPieces[getSquareTo(move)];
            uint16_t oldSpecs     = board.boardSpecs;
            
            bool is_ok_to_reduce = !in_check && !(pv_node && (isCapture(move) || isEnPassant(move) || isPromotion(move)));

            bool is_quiet = (!isCapture(move) && !isPromotion(move) && !isEnPassant(move));

            //get history score of the move
            int history = history_moves[board.colorToMove][oldPieceType][getSquareTo(move)];

            //skip quiet moves
            if (is_quiet && skip_quiet_moves)
                continue;
            

            if (!is_root && !in_check && is_quiet && alpha > -MATE_SCORE) {
                
                /*
                Late move pruning (LMP)

                if the move is quiet and we have already searched 
                enough moves before, we can skip it.
                */
                if (!pv_node && !is_root && quietMoveCount >= LMP_table[improving][depth]) {
                    skip_quiet_moves = true;
                    continue;
                }

                int LMRdepth = LMR_table[std::min(depth, 63)][std::min(moveCount, 63)];


                /*
                Futility pruning
                
                if the move is quiet and the position has low potential 
                of raising alpha, we can skip all following quiet moves
                */
                if (LMRdepth <= 6 && (static_eval + 215 + 70*depth) <= alpha) {
                    skip_quiet_moves = true;
                }

                //SEE pruning for quiets
                if(depth <= 8 && !see(move, -70*depth))
                    continue;
                
            }
            //if not quiet move
            else {
                //SEE pruning for non-quiet moves: if the move leads to a losing exchange, we can skip it
                //Slso, we give a threshold that increases with depth: the idea is that if we are at a high depth (meaning near to the root), even if we lose some material in the exchange we can't be sure enough to prune that branch completely (unless we lose a lot, like a queen) because we are nowhere near to the leaf nodes.
                if(depth <= 6 && !see(move, -15*depth*depth))
                    continue;  
            }



            //Singular extensions and multicut
            
            int extension = 0;
            if(!is_root && ttHit && ttEntry->best_move == move && ttEntry->flag == HASH_FLAG_BETA && depth >= (6 + pv_node) && (ttEntry->depth >= depth - 3) && std::abs(ttEntry->eval) < MATE_SCORE) {
                
                int singular_beta  = ttEntry->eval - depth;
                
                //If we are in a situation where we had a ttHit with BETA FLAG, but the depth wasn't enough to return the value, we can use this info to say that probably we will still fail high.
                //So we do a reduced-depth search STAYING at this level, meaning researching this current position,
                //and if it fails high, we return singular_beta, pruning the tree (multicut)
                //if we don't fail high, we may want to extend the search, because it's an uncertain position
                ss->excluded_move = move;

                int singular_score = search((depth-1) / 2, singular_beta-1, singular_beta, ss);

                ss->excluded_move = NULL_MOVE;

                //if no move fails high, the current move s singular, and we extend the search
                if(singular_score < singular_beta) {
                    extension = 1;

                    //double extension in case move is very singular
                    if(!pv_node && singular_score < singular_beta - 25 && ss->double_extension < 6) {
                        extension = 2;
                        ss->double_extension = (ss-1)->double_extension + 1;
                    }
                }

                //if all other moves fail high, cut
                else if(singular_beta >= beta) return singular_beta; // multicut

                /* 
                    if we didn't prove every move fails high,
                    but our stored eval is yet greater than beta,
                    we are pretty sure that no move in this subtree
                    is great, so we can search to a lower depth
                */
                else if(ttEntry->eval >= beta) extension = -2;

                /*
                    if we didn't prove every move fails high,
                    but our stored eval is yet greater than beta,
                    but lower than the score returned by the null-window search,
                    it means that maybe the eval stored in the TT wasn't so accurate.
                    Therefore, we still trust it, but we reduce the depth of the search
                    in this subtree only by one (so not a lot, because the situation is 
                    more uncertain)
                */
                else if(ttEntry->eval <= singular_score) extension = -1;

            }

            int newDepth = depth + extension;


            // Copy the hash key to restore it after the search, together with unplay move
            uint64_t hash_key_copy = hash_key;
            fillDirtyPiece(ply+1, move);
            
            playMove(move);
            repetition_index++;
            repetition_table[repetition_index] = hash_key;
            ply++;

            nodes++;

            ss->move = move;

            if(is_quiet) {
                quietList.moves[quietMoveCount] = move;
                quietMoveCount++;
            }


            // full depth search
            if (moveCount == 0)
                evaluation = -search(newDepth - 1, -beta, -alpha, ss + 1);

            // Late move reduction (LMR)
            else {
                // condition to consider late move reduction (LMR)
                if ((moveCount >= 3 + improving) && (depth >= 3) && is_ok_to_reduce) {
                    int R = LMR_table[std::min(depth, 63)][std::min(moveCount, 63)];

                    R += !pv_node;   // increase reduction if we it's not a pv-node
                    R += !improving; // increase reduction if we are not improving
                    R += is_quiet && !see(move, -50 * depth);   // we increase the reduction if the move is quiet, because they are less likely to be the best move

                    /*
                    decrease reduction if the move has a high history score
                    (it's more likely to cause a cutoff, so we want to search it deeper)
                    */
                    R -= history / 4000; 

                    R -= 2 * isKillerMove; // if the move is a killer move, we want to search it deeper, therefore we make the reduction smaller

                    
                    R = std::min(newDepth - 1, std::max(1, R)); // make sure we don't end up in quiescence

                    evaluation = -search(newDepth - R, -alpha - 1, -alpha, ss + 1); // search move with a reduced search
                }

                else
                    // hack to ensure that full search is done
                    evaluation = alpha + 1;

                // Principle variation search (PVS)
                if (evaluation > alpha) {
                    //  https://web.archive.org/web/20071030220825/http://www.brucemo.com/compchess/programming/pvs.htm
                    //  principle variation search optimization
                    /*  Once you've found a move with a score that is between alpha and beta,
                        the rest of the moves are searched with the goal of proving that they are all bad.
                        It's possible to do this a bit faster than a search that worries that one
                        of the remaining moves might be good. */
                    evaluation = -search(newDepth - 1, -alpha - 1, -alpha, ss + 1);
                    /*  If the algorithm finds out that it was wrong, and that one of the
                        subsequent moves was better than the first PV move, it has to search again,
                        in the normal alpha-beta manner.  This happens sometimes, and it's a waste of time,
                        but generally not often enough to counteract the savings gained from doing the
                        "bad move proof" search referred to earlier. */
                    if ((evaluation > alpha) && (evaluation < beta))
                        // research the move  that has failed to be proved to be bad
                        evaluation = -search(newDepth - 1, -beta, -alpha, ss + 1);
                }
            }

            ply--;
            repetition_index--;
            hash_key = hash_key_copy;
            unplayMove(move, oldPieceType, oldSpecs, capturedPieceType);

            // reutrn 0 if time is up
            if (stopped)
                return 0;


            if (evaluation > alpha) {
                // switch hahs flag to the one storing score for PV node
                hash_f = HASH_FLAG_EXACT;

                best_move = move;
                    
                alpha = evaluation;

                // write PV move
                pv_table[ply][ply] = move;

                // loop over next ply's pv_table
                for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
                    // copy move from deeper ply into current ply
                    pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];

                // adjust PV length
                pv_length[ply] = pv_length[ply + 1];

                if (evaluation >= beta) {
                    // store hash entry
                    if(excluded_move==NULL_MOVE)
                        writeHashEntry(depth, beta, best_move, HASH_FLAG_BETA);

                    // update killer and history moves (only if it's a quiet move)
                    if (is_quiet) {
                        // update killer moves
                        updateKillers(move);
                        //update history score
                        updateHistoryScore(move, best_move, depth, &quietList, quietMoveCount);
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

static inline void printMove(MOVE move) {
    printf("%s", coordFromPosition[getSquareFrom(move)]); 
    printf("%s", coordFromPosition[getSquareTo(move)]);
    // in case of promotion
    if (isPromotion(move)) {
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


void printPV(int len, int cnt, uint16_t* pv) {
    if(cnt == len) return;

    movesList legalMoves;
    board.calculateMoves(board.colorToMove, &legalMoves);
    bool moveIsLegal = false;
    for(int i = 0; i < legalMoves.count; i++) {
        if(pv[cnt] == legalMoves.moves[i]) {
            moveIsLegal = true;
            break;
        }
    }
    if(!moveIsLegal) return;

    uint64_t hash_key_copy = hash_key;
    int oldPieceType = board.allPieces[getSquareFrom(pv[cnt])];
    int capturedPieceType = board.allPieces[getSquareTo(pv[cnt])];

    uint16_t oldSpecs = board.boardSpecs;
    printMove(pv[cnt]);
    printf(" ");
    playMove(pv[cnt]);
    printPV(len, cnt+1, pv);
    hash_key = hash_key_copy;
    unplayMove(pv[cnt], oldPieceType, oldSpecs, capturedPieceType);
}

void search_position(int maxDepth) {
    int evaluation = 0;
    
    nodes = 0; //reset nodes counter
    // reset follow PV flags
    follow_pv = 0;
    score_pv = 0;
    ply = 0; // set ply to 0
    stopped = 0; // reset "time is up" flag

    memset(killer_moves, 0, sizeof(killer_moves));
    memset(history_moves, 0, sizeof(history_moves));
    memset(pv_table, 0, sizeof(pv_table));
    memset(pv_length, 0, sizeof(pv_length));
    currentAge++; // increase currentAge for hash table at each new search

    int alpha = -inf;
    int beta = inf;

    int delta = ASPIRATION_WINDOW_MAX;

    // iterative deepening
    for (int curr_depth = 1; curr_depth <= maxDepth; curr_depth++) {
        ply = 0;
        //reset computed accumulations
        for(int i = 0; i < max_ply+1; i++)
            nn_stack[i].accumulator.computedAccumulation=0;


        // stop calculating and return best move so far
        if (stopped == 1)   
            break;
        

        follow_pv = 1; // enable follow PV flag

        //Aspiration window

        alpha = evaluation - delta;
        beta  = evaluation + delta;
        if(curr_depth <= 3) {
            alpha = -inf;
            beta  =  inf;
        }

        // search at the current depth
        evaluation = search(curr_depth, alpha, beta, searchStack);
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

        delta = ASPIRATION_WINDOW_MAX; // reset aspiration window size

        // if PV is available
        if (pv_length[0]) {

            int nps = static_cast<int>(1000.0f * nodes / (get_time_ms() - starttime));
            nps = nps < 0 ? 0 : nps;
            // print search info
            if (evaluation > -MATE_VALUE && evaluation < -MATE_SCORE)
                printf("info score mate %d depth %d nodes %d nps %d time %d pv ", -(evaluation + MATE_VALUE) / 2 - 1, curr_depth, nodes, nps, get_time_ms() - starttime);

            else if (evaluation > MATE_SCORE && evaluation < MATE_VALUE)
                printf("info score mate %d depth %d nodes %d nps %d time %d pv ", (MATE_VALUE - evaluation) / 2 + 1, curr_depth, nodes, nps, get_time_ms() - starttime);

            else
                printf("info score cp %d depth %d nodes %d nps %d time %d pv ", evaluation, curr_depth, nodes, nps, get_time_ms() - starttime);

            
            printPV(pv_length[0], 0, &(pv_table[0][0]));

            // print new line
            printf("\n");
        }
    }

    MOVE best_move = pv_table[0][0];
    printf("bestmove ");
    printMove(best_move);
    printf("\n");
    
}