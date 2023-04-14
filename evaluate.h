#ifndef EVALUATE_H
#define EVALUATE_H
#define NOMINMAX
#include <stdint.h>
#include <stdlib.h>
#include "constants.h"
#include "bb_helpers.h"
#include "board_declaration.h"
#include "magic_bitboards.h"
#include "eval_constants.h"

const int maxEndgameWeight = 256;

int endgameWeight;

// function to interpolate between values, to have a smooth transition between mid-game and endgame evaluation
static inline int fade(const int values[2])
{
    // 8 is the maximum endgame weight, 1 is the minimum
    return ((values[0] * (maxEndgameWeight - endgameWeight) + (values[1] * endgameWeight)) / maxEndgameWeight);
    // int output = values[0] + ((values[1] - values[0]) / (4 - 1)) * (endgameWeight - 1);

}

// will be used in positional score function (made globally for memmory optimization)
uint64_t white_occupancy = 0ULL;
uint64_t black_occupancy = 0ULL;
uint64_t occupancy = 0ULL;
uint64_t white_pawn_bb = 0ULL;
uint64_t black_pawn_bb = 0ULL;

uint64_t attackedSquares;
uint64_t xRayAttackedSquares;

int square;
int double_pawns;
uint64_t bb;
uint64_t virtualBishopMobility;
uint64_t virtualRookMobility;
uint64_t virtualMobility;

int knightPhase = 1;
int bishopPhase = 1;
int rookPhase = 2;
int queenPhase = 4;
int totalPhase = 24;

// function that returns a number 0-256 to calculate the game phase we are in
//  the startpos is 0, and when no pieces except pawns and kings are on the board, this score is 256
static inline int calculateEndgameWeight()
{
    int phase = totalPhase;
    phase -= count_bits(board.whiteBitboards[N]) * knightPhase;
    phase -= count_bits(board.blackBitboards[N]) * knightPhase;
    phase -= count_bits(board.whiteBitboards[B]) * bishopPhase;
    phase -= count_bits(board.blackBitboards[B]) * bishopPhase;
    phase -= count_bits(board.whiteBitboards[R]) * rookPhase;
    phase -= count_bits(board.blackBitboards[R]) * rookPhase;
    phase -= count_bits(board.whiteBitboards[Q]) * queenPhase;
    phase -= count_bits(board.blackBitboards[Q]) * queenPhase;

    return (phase * maxEndgameWeight + (totalPhase / 2)) / totalPhase;
}

int whiteAttackingPiecesCount = 0;
int blackAttackingPiecesCount = 0;
int whiteValueOfAttacks = 0;
int blackValueOfAttacks = 0;
int whitePawnCount = 0;
int blackPawnCount = 0;
int file;
int score;
uint64_t whiteKingRing;
uint64_t blackKingRing;
uint64_t excludedFromWhiteMobility;
uint64_t excludedFromBlackMobility;

//space
uint64_t centerFiles = file_masks[2] | file_masks[3] | file_masks[4] | file_masks[5];
uint64_t whiteSpaceMask = centerFiles & (rank_masks[8 * 1] | rank_masks[8 * 2] | rank_masks[8 * 3]);
uint64_t blackSpaceMask = centerFiles & (rank_masks[8 * 6] | rank_masks[8 * 5] | rank_masks[8 * 4]);
uint64_t whiteSafe, blackSafe, whiteBehind, blackBehind;
uint64_t defended, weak, stronglyProtected, nonPawnWhite, nonPawnBlack;
int blockedCount;

int passedRank;
int whiteKingSq, blackKingSq;

static inline int distance(int sq1, int sq2) {
    return std::max(abs((sq1%8)-(sq2%8)), abs(sq1/8 - sq2/8));
} 

static inline int kingProximity(int kingSq, int sq) {
    return std::min(distance(kingSq, sq), 5);
}

// function that evaluates the position in terms of piece activity, piece position and other parameters
static inline int positionalScore()
{
    score = 0;
    endgameWeight = calculateEndgameWeight();
    attackedSquares = 0ULL;
    xRayAttackedSquares = 0ULL;

    white_pawn_bb = board.whiteBitboards[P];
    black_pawn_bb = board.blackBitboards[P];

    white_occupancy = board.whiteBitboards[K] | board.whiteBitboards[Q] | board.whiteBitboards[R] | board.whiteBitboards[B] | board.whiteBitboards[N] | board.whiteBitboards[P];
    black_occupancy = board.blackBitboards[K] | board.blackBitboards[Q] | board.blackBitboards[R] | board.blackBitboards[B] | board.blackBitboards[N] | board.blackBitboards[P];
    occupancy = white_occupancy | black_occupancy;

    board.getTotalAttackedSquares(occupancy);

    //exclude our pawns on rank 2 and 3, our king, squares protected by enemy pawns
    excludedFromWhiteMobility = (white_pawn_bb & (rank_masks[8 * 1] | rank_masks[8 * 2])) | board.whiteBitboards[K] | board.white_attacked_squares_bb[P];
    excludedFromBlackMobility = (black_pawn_bb & (rank_masks[8 * 5] | rank_masks[8 * 6])) | board.blackBitboards[K] | board.black_attacked_squares_bb[P];

    whiteAttackingPiecesCount = 0;
    blackAttackingPiecesCount = 0;
    whiteValueOfAttacks = 0;
    blackValueOfAttacks = 0;

    whiteKingRing = board.white_attacked_squares_bb[K] & ~board.whiteDoubleAttackedByPawn;
    blackKingRing = board.black_attacked_squares_bb[K] & ~board.blackDoubleAttackedByPawn;

    //get king positions
    bb = board.whiteBitboards[K];
    whiteKingSq = pop_lsb(bb);
    bb = board.blackBitboards[K];
    blackKingSq = pop_lsb(bb);

    for (int pieceColor = 0; pieceColor < 2; pieceColor++)
    {
        for (int piece = K; piece <= P; piece++)
        {
            // black
            if (pieceColor == 0)
            {
                bb = board.blackBitboards[piece];
                while (bb)
                {
                    square = pop_lsb(bb);
                    score -= fade(psqt[piece][FLIP(square)]);

                    if (piece == Q)
                        attackedSquares = get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy);
                    else if (piece == R)
                        attackedSquares = get_rook_attacks(square, occupancy);
                    else if (piece == B)
                        attackedSquares = get_bishop_attacks(square, occupancy);
                    else if (piece == N)
                        attackedSquares = knight_attacks[square] & ~black_occupancy;


                    //for queen, rook, bishops and knight
                    if ((piece >= Q) && (piece <= N))
                    {
                        //mobility bonus
                        score -= fade(mobilityBonus[piece][count_bits(attackedSquares & ~excludedFromBlackMobility)]);
                        //check if the piece attacks the enemy king ring
                        if(attackedSquares & whiteKingRing) { 
                            blackAttackingPiecesCount++;
                            blackValueOfAttacks += count_bits(attackedSquares & whiteKingRing) * attackerWeight[piece];
                        }
                        // if rook or bishop xray the king add bonus
                        else if (piece == R && (file_masks[square] & whiteKingRing))
                            score -= fade(rookOnKingRing);
                        else if (piece == B && (get_bishop_attacks(square, (1ULL << square) | board.whiteBitboards[P] | board.blackBitboards[P]) & whiteKingRing))
                            score -= fade(bishopOnKingRing);
                    }


                    switch (piece)
                    {
                    case K:

                        virtualBishopMobility = get_bishop_attacks(square, occupancy);
                        virtualRookMobility = get_rook_attacks(square, occupancy);
                        virtualMobility = virtualRookMobility | virtualBishopMobility;

                        // penalize the king based on the number of squares it can see if it was a queen (the more the king is opened, the more it gets penalized)
                        // the penalty becomes weaker the more we approach the endgame
                        score -= fade(virtualMobilityPenalty[count_bits(virtualMobility)]);

                        // king shield bonus
                        score -= count_bits(king_attacks[square] & black_pawn_bb) * fade(king_pawn_shield_bonus);

                        // if we are in the endgame, give a bonus if the king is far from the corners (bigger the more we are into the endgame)
                        score -= dstToCorner[square] * fade(dst_to_corner_bonus);

                        break;
                    case Q:
                        break;
                    case R:
                        // semi-open file score
                        // if no white pawns on the file
                        if ((black_pawn_bb & file_masks[square]) == 0)
                            score -= semiopen_file_score;
                        // if no pawns at all on the file
                        if (((black_pawn_bb | white_pawn_bb) & file_masks[square]) == 0)
                            score -= open_file_score;

                        break;
                    case B:
                        xRayAttackedSquares = get_bishop_attacks(square, 1ULL<<square);
                        // penalty based on number of our pawns on the same color of our bishop

                        // if it's black-square bishop
                        if (get_bit(black_squares_bb, square))
                            score -= count_bits(black_pawn_bb & black_squares_bb) * fade(penalty_pawns_on_same_bishop_color);
                        // else, if it's a white color bishop
                        else
                            score -= count_bits(black_pawn_bb & white_squares_bb) * fade(penalty_pawns_on_same_bishop_color);

                        // bonus to bishop on long diagonal that can see the center (1 or 2 squares)
                        if (count_bits(attackedSquares & center) == 2)
                            score -= fade(bishop_on_long_diagonal_bonus);

                        score -= fade(bishopXRayPawns) * count_bits(xRayAttackedSquares & white_pawn_bb);

                        //bishop outpost
                        if (((8 <= square) && (square <= 31)) && get_bit(board.black_attacked_squares_bb[P], square) && !((black_passed_mask[square] & ~file_masks[square]) && white_pawn_bb))
                        {
                            score -= fade(outpostBonus);
                        }

                        break;
                    case N:

                        // check if knight is on an outpost
                        // we check it by seeing if the knight is on:
                        //  - rank from 4 to 6 (in respect to the view of each side)
                        //  - a pawn-defended square
                        //  - if it has no pawns that can attack him, meaning if there are no enemy pawns on the 2 files next to the knight file.
                        if (((8 <= square) && (square <= 31)) && get_bit(board.black_attacked_squares_bb[P], square) && !((black_passed_mask[square] & ~file_masks[square]) && white_pawn_bb))
                        {
                            score -= fade(outpostBonus);
                        }
                        break;
                    case P:
                        // double pawn penalty
                        double_pawns = count_bits(file_masks[square] & black_pawn_bb);
                        // penalty if the pawn is doubled
                        if (double_pawns > 1)
                            score -= fade(double_pawn_penalty);

                        // isolated pawn penalty
                        if (!(black_pawn_bb & isolated_masks[square]))
                            score -= fade(isolated_pawn_penalty);

                        // passed pawn bonus
                        if (!(black_passed_mask[square] & white_pawn_bb))
                        {
                            passedRank = get_rank[FLIP(square)];
                            int blockSq = square - 8;

                            score -= fade(passed_pawn_bonus[passedRank]);

                            //bonus based on king proximity
                            if(passedRank > 2) {
                                score -= ((kingProximity(whiteKingSq, blockSq)-1) - (kingProximity(blackKingSq, blockSq))) * passedRank * endgameWeight / 256;
                            }
                            //if the block square is free, increase the advantage
                            if(board.getPieceTypeOnSquare(blockSq) == -1) {
                                score -= fade(freeBlockSquareBonus);
                            }

                            // if the passed pawn is supported by a rook placed on the same file, give a bonus
                            if ((file_masks[square] & board.blackBitboards[R]))
                                score -= fade(supported_passed_pawn_by_rook_bonus);

                            //give penalty if an enemy rook gets on its file
                            if ((file_masks[square] & board.whiteBitboards[R]))
                                score -= fade(attacked_passed_pawn_by_rook_penalty);
                            // give bonus if it is an outside passed pawn
                            if (get_bit((file_masks[0] | file_masks[7]), square))
                                score -= fade(outside_passed_pawn_bonus);

                        }

                        // connected pawns
                        // give bonus if the pawns are connected, meaning if they are next to each other or if they form  a pawn chain

                        // if they are next to each other
                        score -= fade(connected_pawn_bonus) * count_bits((isolated_masks[square] & (rank_masks[square] | rank_masks[square+8] | rank_masks[square-8])) & black_pawn_bb);

                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {

                bb = board.whiteBitboards[piece];
                while (bb)
                {
                    square = pop_lsb(bb);
                    score += fade(psqt[piece][square]);

                    if (piece == Q)
                        attackedSquares = get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy);
                    else if (piece == R)
                        attackedSquares = get_rook_attacks(square, occupancy);
                    else if (piece == B)
                        attackedSquares = get_bishop_attacks(square, occupancy);
                    else if (piece == N)
                        attackedSquares = knight_attacks[square] & ~white_occupancy;

                    
                    if ((piece >= Q) && (piece <= N))
                    {
                        // mobility bonus
                        score += fade(mobilityBonus[piece][count_bits(attackedSquares & ~excludedFromWhiteMobility)]);

                        // check if the piece attacks the enemy king ring
                        if (attackedSquares & blackKingRing) {
                            whiteAttackingPiecesCount++;
                            whiteValueOfAttacks += count_bits(attackedSquares & blackKingRing) * attackerWeight[piece];
                        }
                        //if rook or bishop xray the king add bonus
                        else if (piece == R && (file_masks[square] & blackKingRing))
                            score += fade(rookOnKingRing);
                        else if (piece == B && (get_bishop_attacks(square, (1ULL << square) | board.whiteBitboards[P] | board.blackBitboards[P]) & blackKingRing))
                            score += fade(bishopOnKingRing);
                    }

                    switch (piece)
                    {
                    case K:

                        virtualBishopMobility = get_bishop_attacks(square, occupancy);
                        virtualRookMobility = get_rook_attacks(square, occupancy);
                        virtualMobility = virtualRookMobility | virtualBishopMobility;

                        score += fade(virtualMobilityPenalty[count_bits(virtualMobility)]);

                        // king pawn shield bonus
                        score += count_bits(king_attacks[square] & white_pawn_bb) * fade(king_pawn_shield_bonus);

                        score += dstToCorner[square] * fade(dst_to_corner_bonus);
                        break;
                    case Q:

                        break;
                    case R:

                        // bonus if the rook is on a semi-open file
                        if ((white_pawn_bb & file_masks[square]) == 0)
                            score += semiopen_file_score;

                        // extra bonus if the rook is on an open file
                        if (((white_pawn_bb | black_pawn_bb) & file_masks[square]) == 0)
                            score += open_file_score;

                        break;
                    case B:

                        xRayAttackedSquares = get_bishop_attacks(square, 1ULL<<square);

                        // penalty based on number of same-side pawns on the same color of our bishop

                        // if it's a black-square bishop
                        if (get_bit(black_squares_bb, square))
                            score += count_bits(white_pawn_bb & black_squares_bb) * fade(penalty_pawns_on_same_bishop_color);
                        // else, if it's a white color bishop
                        else
                            score += count_bits(white_pawn_bb & white_squares_bb) * fade(penalty_pawns_on_same_bishop_color);

                        // bonus to bishop on long diagonal that can see the center in x-ray
                        if (count_bits(attackedSquares & center) == 2)
                            score += fade(bishop_on_long_diagonal_bonus);

                        
                        score += fade(bishopXRayPawns) * count_bits(xRayAttackedSquares & black_pawn_bb);

                        //bishop outpost
                        if (((32 <= square) && (square <= 55)) && get_bit(board.white_attacked_squares_bb[P], square) && !((white_passed_mask[square] & ~file_masks[square]) && black_pawn_bb))
                        {
                            score += fade(outpostBonus);
                        }
                        break;
                    case N:

                        // check if knight is on an outpost
                        // we check it by seeing if the knight is on:
                        //  - rank from 4 to 6 (in respect to the view of each side)
                        //  - a pawn-defended square
                        //  - if it has no pawns that can attack him, meaning if there are no enemy pawns on the 2 files next to the knight file.

                        if (((32 <= square) && (square <= 55)) && get_bit(board.white_attacked_squares_bb[P], square) && !((white_passed_mask[square] & ~file_masks[square]) && black_pawn_bb))
                        {
                            score += fade(outpostBonus);
                        }

                        break;
                    case P:
                        // double pawn penalty
                        double_pawns = count_bits(file_masks[square] & white_pawn_bb);
                        // penalty if the pawn is doubled
                        if (double_pawns > 1)
                            score += fade(double_pawn_penalty);

                        // isolated pawn penalty
                        if (!(white_pawn_bb & isolated_masks[square]))
                            score += fade(isolated_pawn_penalty);

                        // passed pawn bonus
                        if (!(white_passed_mask[square] & black_pawn_bb))
                        {
                            passedRank = get_rank[square];
                            int blockSq = square+8;
                            score += fade(passed_pawn_bonus[passedRank]);
                            
                            //bonus based on king proximity
                            if(passedRank > 2) {
                                score += ((kingProximity(blackKingSq, blockSq) - 1) - (kingProximity(whiteKingSq, blockSq))) * passedRank * endgameWeight / 256;
                            }
                            //if the block square is free, increase the advantage
                            if(board.getPieceTypeOnSquare(blockSq) == -1) {
                                score += fade(freeBlockSquareBonus);
                            }

                            // if the passed pawn is supported by a rook placed on the same file, give a bonus
                            if ((file_masks[square] & board.whiteBitboards[R]))
                                score += fade(supported_passed_pawn_by_rook_bonus);

                            // give penalty if an enemy rook gets on its file
                            if ((file_masks[square] & board.blackBitboards[R]))
                                score += fade(attacked_passed_pawn_by_rook_penalty);
                            // give bonus if it is an outside passed pawn
                            if (get_bit((file_masks[0] | file_masks[7]), square))
                                score += fade(outside_passed_pawn_bonus);
                        }

                        // connected pawns
                        // give bonus if the pawns are connected, meaning if they are next to each other or if they form  a pawn chain

                        // if they are next to each other
                        score += fade(connected_pawn_bonus) * count_bits((isolated_masks[square] & (rank_masks[square] | rank_masks[square+8] | rank_masks[square-8])) & white_pawn_bb);

                        break;
                    default:
                        break;
                    }

                }
            }
        }
    }

    nonPawnWhite = white_occupancy & ~board.whiteBitboards[P];
    nonPawnBlack = black_occupancy & ~board.blackBitboards[P];
    stronglyProtected = board.black_attacked_squares_bb[P] | (board.blackDoubleAttacked & ~board.whiteDoubleAttacked);
    // Non-pawn enemies, strongly protected
    defended = nonPawnBlack & stronglyProtected;
    // Enemies not strongly protected and under our attack
    weak = black_occupancy & ~stronglyProtected & board.white_total_attacked_squares;
    // Bonus according to the kind of attacking pieces
    if (defended | weak) {
        bb = (defended | weak) & (board.white_attacked_squares_bb[N] | board.white_attacked_squares_bb[B]);
        while (bb)
            score += fade(threatByMinor[board.getBlackPieceTypeOnSquare(pop_lsb(bb))]);

        bb = weak & board.white_attacked_squares_bb[R];
        while (bb)
            score += fade(threatByRook[board.getBlackPieceTypeOnSquare(pop_lsb(bb))]);

        bb = ~board.black_total_attacked_squares | (nonPawnBlack & board.whiteDoubleAttacked);
        score += fade(hangingBonus) * count_bits(weak & bb);
        // Additional bonus if weak piece is only protected by a queen
        score += fade(weakQueenProtection) * count_bits(weak & board.black_attacked_squares_bb[Q]);
    }

    stronglyProtected = board.white_attacked_squares_bb[P] | (board.whiteDoubleAttacked & ~board.blackDoubleAttacked);
    // Non-pawn enemies, strongly protected
    defended = nonPawnWhite & stronglyProtected;
    // Enemies not strongly protected and under our attack
    weak = white_occupancy & ~stronglyProtected & board.black_total_attacked_squares;
    // Bonus according to the kind of attacking pieces
    if (defended | weak) {
        bb = (defended | weak) & (board.black_attacked_squares_bb[N] | board.black_attacked_squares_bb[B]);
        while (bb)
            score -= fade(threatByMinor[board.getWhitePieceTypeOnSquare(pop_lsb(bb))]);

        bb = weak & board.black_attacked_squares_bb[R];
        while (bb)
            score -= fade(threatByRook[board.getWhitePieceTypeOnSquare(pop_lsb(bb))]);

        bb = ~board.white_total_attacked_squares | (nonPawnWhite & board.blackDoubleAttacked);
        score -= fade(hangingBonus) * count_bits(weak & bb);
        // Additional bonus if weak piece is only protected by a queen
        score -= fade(weakQueenProtection) * count_bits(weak & board.white_attacked_squares_bb[Q]);
    }

    // threat by safe pawn push (bonus for white)
    whiteSafe = ~board.black_total_attacked_squares | board.white_total_attacked_squares;
    bb = ((board.whiteBitboards[P] & ~rank_masks[8*6]) << 8) & ~occupancy;
    bb |= ((bb & rank_masks[8*2]) << 8) & ~occupancy;
    bb &= ~board.black_attacked_squares_bb[P] & whiteSafe;
    bb = board.whitePawnsAttacks(bb) & (black_occupancy & ~board.blackBitboards[P]);
    score += fade(threatByPawnPush) * count_bits(bb);

    // threat by safe pawn push (bonus for black)
    blackSafe = ~board.white_total_attacked_squares | board.black_total_attacked_squares;
    bb = ((board.blackBitboards[P] & ~rank_masks[8 * 1]) >> 8) & ~occupancy;
    bb |= ((bb & rank_masks[8 * 5]) >> 8) & ~occupancy;
    bb &= ~board.white_attacked_squares_bb[P] & blackSafe;
    bb = board.blackPawnsAttacks(bb) & (white_occupancy & ~board.whiteBitboards[P]);
    score -= fade(threatByPawnPush) * count_bits(bb);

    //minor behind pawn bonuses
    score += count_bits((white_pawn_bb >> 8) & (board.whiteBitboards[N] | board.whiteBitboards[B])) * fade(minorBehindPawnBonus);
    score -= count_bits((black_pawn_bb << 8) & (board.blackBitboards[N] | board.blackBitboards[B])) * fade(minorBehindPawnBonus);

    //attack bonuses
    score += (whiteValueOfAttacks*attackWeight[whiteAttackingPiecesCount] / 100);
    score -= (blackValueOfAttacks*attackWeight[blackAttackingPiecesCount] / 100);

    // bonus to the side which has the pair of bishops
    if ((count_bits(board.blackBitboards[B]) - count_bits(board.whiteBitboards[B])) == 2)
        score -= bishop_pair_bonus;
    else if ((count_bits(board.whiteBitboards[B]) - count_bits(board.blackBitboards[B])) == 2)
        score += bishop_pair_bonus;

    return score;
}

// evaluation function
// counts material and evaluates the positional score
static inline int evaluate()
{
    int evaluation = board.whitePiecesValue - board.blackPiecesValue;

    evaluation += positionalScore();

    int perspective = board.colorToMove ? 1 : -1;

    return evaluation * perspective;
}

#endif