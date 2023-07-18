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

// function to interpolate between values, to have a smooth transition between midgame and endgame evaluation
static inline int fade(const int values[2])
{
    return ((values[0] * (maxEndgameWeight - endgameWeight) + (values[1] * endgameWeight)) / maxEndgameWeight);
}

//fades between non constant values, calcukated in the eval function
static inline int fade_onfly(int mg, int eg)
{
    return ((mg * (maxEndgameWeight - endgameWeight) + (eg * endgameWeight)) / maxEndgameWeight);
}

uint64_t occupancy = 0ULL;

uint64_t attackedSquares;
uint64_t xRayAttackedSquares;

int square, relativeSquare;
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
//  the startpos is 0, and when no pieces except pawns and kings are on the board, the endgameWeight is 256
static inline int calculateEndgameWeight()
{
    int phase = totalPhase;
    phase -= count_bits(pieces_bb[WHITE][N]) * knightPhase;
    phase -= count_bits(pieces_bb[BLACK][N]) * knightPhase;
    phase -= count_bits(pieces_bb[WHITE][B]) * bishopPhase;
    phase -= count_bits(pieces_bb[BLACK][B]) * bishopPhase;
    phase -= count_bits(pieces_bb[WHITE][R]) * rookPhase;
    phase -= count_bits(pieces_bb[BLACK][R]) * rookPhase;
    phase -= count_bits(pieces_bb[WHITE][Q]) * queenPhase;
    phase -= count_bits(pieces_bb[BLACK][Q]) * queenPhase;

    return (phase * maxEndgameWeight + (totalPhase / 2)) / totalPhase;
}
int file, score, passedRank;
uint64_t defended, weak, stronglyProtected;

static inline int distance(int sq1, int sq2)
{
    return std::max(abs((sq1 % 8) - (sq2 % 8)), abs(sq1 / 8 - sq2 / 8));
}

static inline int kingProximity(int kingSq, int sq)
{
    return std::min(distance(kingSq, sq), 5);
}

uint64_t *passed_mask;
int Us = 1;
int Them = 0;
int Up;
uint64_t excludedFromMobility, mobilityArea, lowRanks, spaceMask, behind, blockedPawns;
uint64_t enemyKingRing, ourKingRing;
int attackingPiecesCount, valueOfAttacks;
int ourKingSq, enemyKingSq;
int bonus;

uint64_t nonPawnEnemies, safe;
uint64_t squaresToQueen, unsafeSquares;

uint64_t passed, opposed, blocked, stoppers, lever, leverPush, doubled, neighbours, phalanx, support, backward, ourPawns, theirPawns;

static inline int get_relative_square(int square)
{
    if (Us == 1)
        return square;
    return FLIP(square);
}

uint64_t Shift(uint64_t bb, int dir)
{
    return (dir > 0) ? (bb << dir) : (bb >> -dir);
}

bool bool_val(uint64_t bb)
{
    return bb ? 1 : 0;
}

int more_than_one(uint64_t bb)
{
    pop_lsb(bb);
    return !(bb == 0);
}

bool isPassedPawn(int s)
{

    int rank = get_relative_square(s) / 8;

    ourPawns = pieces_bb[Us][P];
    theirPawns = pieces_bb[Them][P];

    opposed = theirPawns & passed_mask[s] & file_masks[s];
    blocked = theirPawns & (1ULL << (s + Up));
    stoppers = theirPawns & passed_mask[s];
    lever = theirPawns & board.pawnsAttacks(1ULL << s, Us);
    leverPush = theirPawns & board.pawnsAttacks(1ULL << (s + Up), Us);
    doubled = ourPawns & (1ULL << (s - Up));
    neighbours = ourPawns & isolated_masks[s];
    phalanx = neighbours & rank_masks[s];
    support = neighbours & rank_masks[s - Up];

    passed = !(stoppers ^ lever) || (!(stoppers ^ leverPush) && count_bits(phalanx) >= count_bits(leverPush)) || (stoppers == blocked && (rank >= 4) && (Shift(support, Up) & ~(theirPawns | double_attacked_by_pawn[Them])));

    passed &= !(passed_mask[s] & file_masks[s] & ourPawns);

    if (support | phalanx)
        score += count_bits(support) * fade(supportPawnBanus) + count_bits(phalanx) * fade(phalanxPawnBouns);

    if (passed)
        return true;
    return false;
}

static inline int evaluateSide()
{
    score = 0;
    attackingPiecesCount = 0;
    valueOfAttacks = 0;

    occupancy = pieces_bb[Us][ALL_PIECES] | pieces_bb[Them][ALL_PIECES];

    excludedFromMobility = pieces_bb[Us][P] & (rank_masks[get_relative_square(8)] | rank_masks[get_relative_square(16)]) | pieces_bb[Us][K] | attacked_squares[Us][P];

    enemyKingRing = attacked_squares[Them][K] & ~double_attacked_by_pawn[Them];
    ourKingRing = attacked_squares[Us][K] & ~double_attacked_by_pawn[Us];

    bb = pieces_bb[Us][K];
    ourKingSq = pop_lsb(bb);
    bb = pieces_bb[Them][K];
    enemyKingSq = pop_lsb(bb);

    for (int piece = K; piece <= P; piece++)
    {
        bb = pieces_bb[Us][piece];
        while (bb)
        {
            square = pop_lsb(bb);
            relativeSquare = get_relative_square(square);
            score += fade(psqt[piece][relativeSquare]);
            if (piece == Q)
                attackedSquares = get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy);
            else if (piece == R)
                attackedSquares = get_rook_attacks(square, occupancy);
            else if (piece == B)
                attackedSquares = get_bishop_attacks(square, occupancy);
            else if (piece == N)
                attackedSquares = knight_attacks[square] & ~pieces_bb[Us][ALL_PIECES];

            if ((piece >= Q) && (piece <= N))
            {
                // mobility bonus
                score += fade(mobilityBonus[piece][count_bits(attackedSquares & ~excludedFromMobility)]);
                // check if the piece attacks the enemy king ring
                if (attackedSquares & enemyKingRing)
                {
                    attackingPiecesCount++;
                    valueOfAttacks += count_bits(attackedSquares & enemyKingRing) * attackerWeight[piece];
                }
                // if rook or bishop xray the king ring add bonus
                else if ((piece == R) && (file_masks[square] & enemyKingRing))
                    score += fade(rookOnKingRing);
                else if ((piece == B) && (get_bishop_attacks(square, occupancy) & enemyKingRing))
                    score += fade(bishopOnKingRing);
            }

            switch (piece)
            {
            case K:
                virtualBishopMobility = get_bishop_attacks(square, occupancy);
                virtualRookMobility = get_rook_attacks(square, occupancy);
                virtualMobility = virtualRookMobility | virtualBishopMobility;

                score += fade(virtualMobilityPenalty[count_bits(virtualMobility)]);

                score += count_bits(king_attacks[square] & pieces_bb[Us][P]) * fade(king_pawn_shield_bonus);

                // penalty if king is on a pawnless flank (v1.1)
                if (!((pieces_bb[Us][P] | pieces_bb[Them][P]) & kingFlank[ourKingSq % 8]))
                    score -= fade(pawnlessFlank);

                break;

            case Q:
                break;
            case R:
                if ((pieces_bb[Us][P] & file_masks[square]) == 0)
                    score += semiopen_file_score;
                // if no pawns at all on the file
                if (((pieces_bb[Us][P] | pieces_bb[Them][P]) & file_masks[square]) == 0)
                    score += open_file_score;

                break;

            case B:
                xRayAttackedSquares = get_bishop_attacks(square, 1ULL << square);
                // penalty based on number of our pawns on the same color of our bishop

                // if it's black-square bishop
                if (get_bit(black_squares_bb, square))
                    score -= count_bits(pieces_bb[Us][P] & black_squares_bb) * fade(bishopPawns);
                // else, if it's a white color bishop
                else
                    score -= count_bits(pieces_bb[Us][P] & white_squares_bb) * fade(bishopPawns);

                // bonus to bishop on long diagonal that can see the center (1 or 2 squares)
                if (count_bits(attackedSquares & center) == 2)
                    score += fade(bishopOnLongDiagonal);

                score += fade(bishopXRayPawns) * count_bits(xRayAttackedSquares & pieces_bb[Them][P]);

                break;
            case N:
                break;
            case P:
                // double pawn penalty
                double_pawns = count_bits(file_masks[square] & pieces_bb[Us][P]);
                // penalty if the pawn is doubled
                if (double_pawns > 1)
                    score += fade(double_pawn_penalty);

                // isolated pawn penalty
                if (!(pieces_bb[Us][P] & isolated_masks[square]))
                    score += fade(isolated_pawn_penalty);

                // passed pawn bonus

                if (isPassedPawn(square))
                {
                    passedRank = get_rank[relativeSquare];
                    int blockSq = square + Up;

                    bonus = fade(passed_pawn_bonus[passedRank]);

                    // bonus based on king proximity
                    if (passedRank > 2)
                    {

                        int w = 5 * passedRank - 13;
                        bonus += ((kingProximity(enemyKingSq, blockSq) * 19 / 4) - (kingProximity(ourKingSq, blockSq) * 2)) * w * endgameWeight / 256;

                        // if the block square is free, increase the advantage
                        if (board.getPieceTypeOnSquare(blockSq) == -1)
                        {
                            squaresToQueen = file_masks[square] & passed_mask[square];
                            unsafeSquares = passed_mask[square];

                            uint64_t b = file_masks[square] & ~unsafeSquares & (pieces_bb[Us][R] | pieces_bb[Us][Q] | pieces_bb[Them][R] | pieces_bb[Them][Q]);

                            if (!(pieces_bb[Them][ALL_PIECES] & b))
                                unsafeSquares &= attacked_squares[Them][ALL_PIECES] | pieces_bb[Them][ALL_PIECES];

                            int k = !unsafeSquares ? 36 : !(unsafeSquares & ~attacked_squares[Us][P]) ? 30
                                                      : !(unsafeSquares & squaresToQueen)             ? 17
                                                      : !(get_bit(unsafeSquares, blockSq))            ? 7
                                                                                                      : 0;

                            if ((pieces_bb[Us][ALL_PIECES] & b) || get_bit(attacked_squares[Us][ALL_PIECES], blockSq))
                                k += 5;

                            bonus += k * w;
                        }
                    }
                    score += bonus - (fade(passedFile) * std::min(square % 8, 7 - square % 8));
                }

                // connected pawns
                // give bonus if the pawns are connected, meaning if they are next to each other or if they form  a pawn chain

                // if they are next to each other
                // score += fade(connected_pawn_bonus) * count_bits((isolated_masks[square] & (rank_masks[square] | rank_masks[square + Up] | rank_masks[square - Up])) & pieces_bb[Us][P]);

                break;
            default:
                break;
            }
        }
    }
    nonPawnEnemies = pieces_bb[Them][ALL_PIECES] & ~pieces_bb[Them][P];
    stronglyProtected = attacked_squares[Them][P] | (double_attacked[Them] & ~double_attacked[Us]);
    defended = nonPawnEnemies & stronglyProtected;
    weak = pieces_bb[Them][ALL_PIECES] & ~stronglyProtected & attacked_squares[Us][ALL_PIECES];

    // Threats + weak queen protection + hanging
    if (defended | weak)
    {
        bb = (defended | weak) & (attacked_squares[Us][N] | attacked_squares[Us][B]);
        while (bb)
            score += fade(threatByMinor[board.getPieceTypeOnSquare(pop_lsb(bb))]);

        bb = weak & attacked_squares[Us][R];
        while (bb)
            score += fade(threatByRook[board.getPieceTypeOnSquare(pop_lsb(bb))]);

        if (weak & attacked_squares[Us][K])
            score += fade(threatByKing);

        bb = ~attacked_squares[Them][ALL_PIECES] | (nonPawnEnemies & double_attacked[Us]);
        score += fade(hangingBonus) * count_bits(weak & bb);
        // Additional bonus if weak piece is only protected by a queen
        score += fade(weakQueenProtection) * count_bits(weak & attacked_squares[Them][Q]);
    }

    safe = ~attacked_squares[Them][ALL_PIECES] | attacked_squares[Us][ALL_PIECES];

    // Threat by safe pawn
    bb = pieces_bb[Us][P] & safe;
    bb = board.pawnsAttacks(bb, Us) & nonPawnEnemies;
    score += fade(threatBySafePawn) * count_bits(bb);

    // Threat by pawn push
    bb = Shift(pieces_bb[Us][P] & ~rank_masks[get_relative_square(48)], Up) & ~occupancy;
    bb |= Shift(bb & rank_masks[get_relative_square(16)], Up) & ~occupancy;
    bb &= ~attacked_squares[Them][P] & safe;
    bb = board.pawnsAttacks(bb, Us) & nonPawnEnemies;
    score += fade(threatByPawnPush) * count_bits(bb);

    // space evaluation (v1.1)

    spaceMask = (Us == WHITE ? CenterFiles & (rank_masks[8] | rank_masks[16] | rank_masks[24])
                             : CenterFiles & (rank_masks[48] | rank_masks[40] | rank_masks[32]));

    safe = spaceMask & ~pieces_bb[Us][P] & ~attacked_squares[Them][P];
    behind = pieces_bb[Us][P];
    behind |= Shift(behind, -Up);
    behind |= Shift(behind, -Up - Up);
    int bonus = count_bits(safe) + count_bits(behind & safe & ~attacked_squares[Them][ALL_PIECES]);

    blockedPawns = Shift(pieces_bb[Us][P], Up) & (pieces_bb[Them][P] | double_attacked_by_pawn[Them]);

    int weight = count_bits(pieces_bb[WHITE][ALL_PIECES]) - 3 + std::min(count_bits(blockedPawns), 9);
    score += (bonus * weight * weight * (maxEndgameWeight - endgameWeight)) / 12288;

    // minor behind pawn
    score += count_bits(Shift(pieces_bb[Us][P], -Up) & (pieces_bb[Us][N] | pieces_bb[Us][B])) * fade(minorBehindPawnBonus);

    // attack evaluation(v1.2 heavily inspired by Loki chess engine https://github.com/BimmerBass/Loki)
    int attackEvalMg = 0, attackEvalEg = 0;
    if(attackingPiecesCount > 2 || (attackingPiecesCount > 1 && pieces_bb[Us][Q]!=0)) {
        attackEvalMg += valueOfAttacks * attackWeight[std::clamp(attackingPiecesCount, 0, 6)][0] / 100;
        attackEvalEg += valueOfAttacks * attackWeight[std::clamp(attackingPiecesCount, 0, 6)][1] / 100;
        // score += (valueOfAttacks * attackWeight[attackingPiecesCount] / 100);
    }
    score += fade_onfly(attackEvalMg*std::max(0, attackEvalMg) / 256, attackEvalEg*std::max(0,attackEvalEg)/64);


    if ((count_bits(pieces_bb[Us][B]) - count_bits(pieces_bb[Them][B])) == 2)
        score += bishop_pair_bonus;

    return score;
}

static inline int evaluate()
{
    int evaluation = 0;
    endgameWeight = calculateEndgameWeight();
    // Us == 1 means we are white, 0 means we are black
    Us = board.colorToMove == WHITE ? WHITE : BLACK;
    Them = Us == WHITE ? BLACK : WHITE;

    pieces_bb[WHITE][ALL_PIECES] = pieces_bb[WHITE][K] | pieces_bb[WHITE][Q] | pieces_bb[WHITE][R] | pieces_bb[WHITE][B] | pieces_bb[WHITE][N] | pieces_bb[WHITE][P];
    pieces_bb[BLACK][ALL_PIECES] = pieces_bb[BLACK][K] | pieces_bb[BLACK][Q] | pieces_bb[BLACK][R] | pieces_bb[BLACK][B] | pieces_bb[BLACK][N] | pieces_bb[BLACK][P];

    occupancy = pieces_bb[WHITE][ALL_PIECES] | pieces_bb[BLACK][ALL_PIECES];

    board.getTotalAttackedSquares(occupancy);

    passed_mask = (Us == WHITE) ? white_passed_mask : black_passed_mask;

    Up = (Us == WHITE) ? 8 : -8;

    evaluation += evaluateSide();
    Us = (Us == WHITE) ? BLACK : WHITE;
    Them = (Us == WHITE) ? BLACK : WHITE;
    passed_mask = (Us == WHITE) ? white_passed_mask : black_passed_mask;
    Up = -Up;
    evaluation -= evaluateSide();

    int perspective = board.colorToMove ? 1 : -1;
    // evaluation += fade(tempoBonus) * perspective;
    return evaluation + (board.whitePiecesValue - board.blackPiecesValue) * perspective;
}

#endif