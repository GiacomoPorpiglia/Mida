#define NOMINMAX
#include "evaluate.h"
#include <stdint.h>
#include <stdlib.h>

#include "constants.h"
#include "bitboard.h"
#include "board_declaration.h"
#include "magic_bitboards.h"
#include "nnue_eval.h"
#include "nnue.h"
#include "game_constants.h"

const int maxEndgameWeight = 256;
int endgameWeight = 0;
uint64_t occupancy=0, attackedSquares=0,xRayAttackedSquares = 0;
int square=0, relativeSquare=0, double_pawns=0;
uint64_t bb=0, virtualBishopMobility=0,virtualRookMobility=0, virtualMobility=0;
const int knightPhase = 1;
const int bishopPhase = 1;
const int rookPhase = 2;
const int queenPhase = 4;
const int totalPhase = 24;
int Us = 1;
int Them = 0;
int Up = 0;
int file = 0, score = 0, passedRank = 0;
uint64_t defended=0, weak=0, stronglyProtected = 0;
uint64_t *passed_mask = NULL;
uint64_t excludedFromMobility=0, mobilityArea=0, lowRanks=0, spaceMask= 0, behind=0, blockedPawns=0, enemyKingRing=0, ourKingRing= 0, passed=0, opposed=0, blocked=0, stoppers=0, lever=0, leverPush=0, doubled =0, neighbours=0, phalanx=0, support=0, backward=0, ourPawns=0, theirPawns=0, squaresToQueen=0, unsafeSquares=0, nonPawnEnemies=0, safe=0;

int attackingPiecesCount=0, valueOfAttacks=0, ourKingSq=0, enemyKingSq=0, bonus = 0;

uint64_t attacked_squares[2][7];
uint64_t double_attacked_by_pawn[2];
uint64_t double_attacked[2];
const int supportPawnBanus[2] = {3, 6};
const int phalanxPawnBouns[2] = {2, 4};

const int isolated_pawn_penalty[2] = {-15, -30};
const int double_pawn_penalty[2] = {-15, -30};

const int passed_pawn_bonus[8][2] = {{0, 0}, {2, 38}, {15, 36}, {22, 50}, {64, 81}, {166, 184}, {284, 269}, {0, 0}}; // for rank
const int passedFile[2] = {13, 8};


const int supported_passed_pawn_by_rook_bonus[2] = {0, 35};
const int attacked_passed_pawn_by_rook_penalty[2] = {0, -35};
const int outside_passed_pawn_bonus[2] = {0, 20};
const int freeBlockSquareBonus[2] = {0, 30};

const int weakQueenProtection[2] = {7, 0};
const int hangingBonus[2] = {15, 5};
const int threatByMinor[6][2] = {
    {0, 0},
    {20, 40},
    {25, 30},
    {20, 13},
    {15, 12},
    {1, 10}
};
const int threatByRook[6][2] = {
    {0, 0},
    {20, 10},
    {0, 15},
    {11, 15},
    {9, 14},
    {1, 12}
};

//bonus for rook on open/semiopen file
const int semiopen_file_score = 15;
const int open_file_score = 20;

//bonus for each shield pawn
const int king_pawn_shield_bonus[2] = {15, 0};
//penalty if king is on pawnless flank
const int pawnlessFlank[2] = {8, 45};
//penalty for each pawn on same color as bishop
const int bishopPawns[2] = {3, 9};
//penalty for each enemy pawn the bishop x-rays
const int bishopXRayPawns[2] = {-4, -5};
//bonus if bishop is on long diagonal
const int bishopOnLongDiagonal[2] = {15, 0};
//bonus for the side with the bishop pair
const int bishop_pair_bonus = 20;

//bonus for outpost (knight/bishop)
const int outpostBonus[2] = {45, 20};
//bonus if minor piee is behind friendly pawn
const int minorBehindPawnBonus[2] = {10, 3};


//bonus if rook or bishop attacks a square on the enemy king ring
const int rookOnKingRing[2] = {13, 0};
const int bishopOnKingRing[2] = {10, 0};

//bonus if pawns threat enemy pieces 
const int threatByPawnPush[2] = {25, 20}; // 25 0
const int threatBySafePawn[2] = {75, 45};
//bonus for king threats
const int threatByKing[2] = {11, 40};

//stockfish
const int psqt[6][64][2] = {
    {{129, 1}, {156, 21}, {129, 41}, {94, 36}, {94, 36}, {129, 41}, {156, 21}, {129, 1}, 
    {132, 25}, {144, 48}, {111, 64}, {85, 64}, {85, 64}, {111, 64}, {144, 48}, {132, 25},
    {93, 42}, {123, 62}, {81, 81}, {57, 84}, {57, 84}, {81, 81}, {123, 62}, {93, 42},
    {78, 49}, {91, 74}, {66, 82}, {46, 82}, {46, 82}, {66, 82}, {91, 74}, {78, 49},
    {74, 46}, {85, 79}, {50, 95}, {34, 95}, {34, 95}, {50, 95}, {85, 79}, {74, 46},
    {59, 44}, {69, 82}, {39, 88}, {15, 91}, {15, 91}, {39, 88}, {69, 82}, {59, 44},
    {42, 22}, {57, 58}, {31, 55}, {16, 62}, {16, 62}, {31, 55}, {57, 58}, {42, 22}, 
    {28, 5}, {42, 28}, {21, 35}, {-1, 37}, {-1, 37}, {21, 35}, {42, 28}, {28, 5},
    },
    {{1, -33}, {-2, -27}, {-2, -22}, {2, -12}, {2, -12}, {-2, -22}, {-2, -27}, {1, -33},
    {-1, -26}, {2, -15}, {4, -11}, {6, -2}, {6, -2}, {4, -11}, {2, -15}, {-1, -26},
    {-1, -19}, {3, -9}, {6, -4}, {4, 1}, {4, 1}, {6, -4}, {3, -9}, {-1, -19},
    {2, -11}, {2, -1}, {4, 6}, {4, 11}, {4, 11}, {4, 6}, {2, -1}, {2, -11},
    {0, -14}, {6, -3}, {6, 4}, {2, 10}, {2, 10}, {6, 4}, {6, -3}, {0, -14},
    {-2, -18}, {5, -9}, {3, -5}, {4, 1}, {4, 1}, {3, -5}, {5, -9}, {-2, -18}, 
    {-2, -24}, {3, -13}, {5, -11}, {4, -4}, {4, -4}, {5, -11}, {3, -13}, {-2, -24},
    {-1, -35}, {-1, -25}, {1, -21}, {-1, -16}, {-1, -16}, {1, -21}, {-1, -25}, {-1, -35},
    },
    {{-15, -4}, {-9, -6}, {-6, -5}, {-2, -4}, {-2, -4}, {-6, -5}, {-9, -6}, {-15, -4},
    {-10, -6}, {-6, -4}, {-4, -1}, {3, -1}, {3, -1}, {-4, -1}, {-6, -4}, {-10, -6},
    {-12, 3}, {-5, -4}, {-1, -1}, {1, -3}, {1, -3}, {-1, -1}, {-5, -4}, {-12, 3},
    {-6, -3}, {-2, 1}, {-2, -4}, {-3, 4}, {-3, 4}, {-2, -4}, {-2, 1}, {-6, -3},
    {-13, -2}, {-7, 4}, {-2, 4}, {1, -3}, {1, -3}, {-2, 4}, {-7, 4}, {-13, -2}, 
    {-11, 3}, {-1, 1}, {3, -4}, {6, 5}, {6, 5}, {3, -4}, {-1, 1}, {-11, 3},
    {-1, 2}, {6, 2}, {8, 9}, {9, -2}, {9, -2}, {8, 9}, {6, 2}, {-1, 2},
    {-8, 9}, {-9, 0}, {-1, 9}, {4, 6}, {4, 6}, {-1, 9}, {-9, 0}, {-8, 9},
    },
    {{-18, -19}, {-2, -10}, {-3, -12}, {-8, -4}, {-8, -4}, {-3, -12}, {-2, -10}, {-18, -19},
    {-5, -12}, {3, -4}, {6, -6}, {1, 1}, {1, 1}, {6, -6}, {3, -4}, {-5, -12},
    {-2, -5}, {7, -1}, {-2, -1}, {6, 4}, {6, 4}, {-2, -1}, {7, -1}, {-2, -5},
    {-2, -6}, {4, -2}, {9, 0}, {13, 6}, {13, 6}, {9, 0}, {4, -2}, {-2, -6},
    {-4, -6}, {9, -1}, {7, -5}, {11, 5}, {11, 5}, {7, -5}, {9, -1}, {-4, -6}, 
    {-5, -10}, {2, 2}, {1, 1}, {4, 2}, {4, 2}, {1, 1}, {2, 2}, {-5, -10},
    {-6, -11}, {-5, -6}, {2, -1}, {0, 1}, {0, 1}, {2, -1}, {-5, -6}, {-6, -11},
    {-16, -15}, {1, -14}, {-5, -12}, {-8, -8}, {-8, -8}, {-5, -12}, {1, -14}, {-16, -15},
    },
    {{-84, -46}, {-44, -31}, {-35, -24}, {-35, -10}, {-35, -10}, {-35, -24}, {-44, -31}, {-84, -46},
    {-36, -32}, {-19, -26}, {-13, -9}, {-7, 4}, {-7, 4}, {-13, -9}, {-19, -26}, {-36, -32},
    {-29, -19}, {-8, -13}, {3, -4}, {6, 14}, {6, 14}, {3, -4}, {-8, -13}, {-29, -19},
    {-16, -16}, {4, -1}, {19, 6}, {24, 14}, {24, 14}, {19, 6}, {4, -1}, {-16, -16},
    {-16, -21}, {6, -8}, {21, 4}, {24, 19}, {24, 19}, {21, 4}, {6, -8}, {-16, -21}, 
    {-4, -24}, {11, -21}, {28, -8}, {25, 8}, {25, 8}, {28, -8}, {11, -21}, {-4, -24},
    {-32, -33}, {-13, -24}, {2, -24}, {18, 6}, {18, 6}, {2, -24}, {-13, -24}, {-32, -33},
    {-96, -48}, {-39, -42}, {-26, -26}, {-12, -8}, {-12, -8}, {-26, -26}, {-39, -42}, {-96, -48},
    },
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {1, -4}, {2, -3}, {5, 4}, {9, 2}, {8, 8}, {10, 3}, {4, -3}, {-1, -9},
    {-4, -4}, {-7, -4}, {5, -5}, {7, 2}, {12, 1}, {11, 1}, {3, -4}, {-9, -2}, 
    {-1, 4}, {-9, 1}, {4, -4}, {9, -1}, {19, -6}, {8, -6}, {1, -5}, {-2, -3},
    {5, 6}, {-2, 3}, {-5, 1}, {1, -3}, {5, -2}, {0, -2}, {-6, 6}, {2, 4},
    {1, 13}, {-5, 9}, {-3, 9}, {11, 14}, {-4, 14}, {-2, 4}, {-6, 4}, {-5, 6},
    {-4, -1}, {3, -6}, {-1, 6}, {-5, 11}, {2, 11}, {-6, 8}, {5, 4}, {-4, 4},
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    }
};

// bonus to each piece according to how many squares they can go to
constexpr int mobilityBonus[][32][2] = {
    {}, // King

    {{-14, -24}, {-8, -14}, {-4, -4}, {-4, 8}, {9, 19}, {12, 27}, {11, 29}, {18, 36}, {20, 38}, {27, 47}, {32, 47}, {34, 50}, {34, 62}, {35, 64}, {35, 66}, {35, 66}, {35, 68}, {36, 70}, {37, 73}, {38, 74}, {45, 76}, {52, 84}, {52, 85}, {53, 85}, {56, 89}, {57, 92}, {57, 93}, {59, 110}}, // Queen

    {{-30, -41}, {-12, -7}, {0, 8}, {1, 21}, {2, 36}, {7, 50}, {10, 51}, {15, 61}, {20, 66}, {20, 69}, {20, 76}, {22, 80}, {28, 82}, {29, 85}, {33, 87}}, // Rook

    {{-23, -29}, {-10, -12}, {7, -4}, {14, 6}, {19, 10}, {26, 20}, {26, 28}, {30, 29}, {31, 32}, {34, 36}, {39, 39}, {41, 43}, {45, 44}, {48, 49}}, // Bishop

    {{-31, -39}, {-26, -28}, {-6, -15}, {-1, -8}, {1, 3}, {6, 6}, {10, 8}, {14, 10}, {18, 13}} // Knight
};

// penalty based onhow many squares the king sees (meaning that it is more exposed). To calculate the squarees seen by the king, the king is seen as a queen, therefore seeing in all 8 directions
const int virtualMobilityPenalty[28][2] = {{0, 0}, {0, 0}, {-5, 0}, {-8, 0}, {-18, 0}, {-25, 0}, {-29, 0}, {-37, 0}, {-41, 0}, {-54, 0}, {-65, 0}, {-68, 0}, {-69, 0}, {-70, 0}, {-70, 0}, {-70, 0}, {-71, 0}, {-72, 0}, {-74, 0}, {-76, 0}, {-90, 0}, {-104, 0}, {-105, 0}, {-106, 0}, {-112, 0}, {-114, 0}, {-114, 0}, {-119, 0}};

//weight of the attacker indexed by piece type 
const int attackerWeight[6] = {
    0, 80, 40, 20, 20, 0
};
//attackWeight indexed by number of attackers
const int attackWeight[7][2] = {
    {0,0},
    {53,51},
    {84,70},
    {97,87},
    {87,97},
    {90, 102},
    {96, 100}
};






// function to interpolate between values, to have a smooth transition between midgame and endgame evaluation
static inline int fade(const int values[2])
{
    return ((values[0] * (maxEndgameWeight - endgameWeight) + (values[1] * endgameWeight)) / maxEndgameWeight);
}

// fades between non constant values, calcukated in the eval function
static inline int fade_onfly(int mg, int eg)
{
    return ((mg * (maxEndgameWeight - endgameWeight) + (eg * endgameWeight)) / maxEndgameWeight);
}

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


static inline int distance(int sq1, int sq2)
{
    return std::max(abs((sq1 % 8) - (sq2 % 8)), abs(sq1 / 8 - sq2 / 8));
}

static inline int kingProximity(int kingSq, int sq)
{
    return std::min(distance(kingSq, sq), 5);
}

static inline int get_relative_square(int square)
{
    if (Us == WHITE)
        return square;
    return FLIP(square);
}

static inline uint64_t Shift(uint64_t bb, int dir)
{
    return (dir > 0) ? (bb << dir) : (bb >> -dir);
}

static inline bool bool_val(uint64_t bb)
{
    return bb ? 1 : 0;
}

static inline int more_than_one(uint64_t bb)
{
    pop_lsb(bb);
    return !(bb == 0);
}

static inline bool isPassedPawn(int s)
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
    if (attackingPiecesCount > 2 || (attackingPiecesCount > 1 && pieces_bb[Us][Q] != 0))
    {
        attackEvalMg += valueOfAttacks * attackWeight[std::clamp(attackingPiecesCount, 0, 6)][0] / 100;
        attackEvalEg += valueOfAttacks * attackWeight[std::clamp(attackingPiecesCount, 0, 6)][1] / 100;
        // score += (valueOfAttacks * attackWeight[attackingPiecesCount] / 100);
    }
    score += fade_onfly(attackEvalMg * std::max(0, attackEvalMg) / 256, attackEvalEg * std::max(0, attackEvalEg) / 64);

    if ((count_bits(pieces_bb[Us][B]) - count_bits(pieces_bb[Them][B])) == 2)
        score += bishop_pair_bonus;

    return score;
}



int nnue_pieces[33];
int nnue_squares[33];

static inline void nnue_input(int *pieces, int *squares) {
    uint64_t bb;
    int piece, square, idx=2;
    //white king
    bb = pieces_bb[WHITE][K];
    pieces[0] = 1;
    squares[0] = pop_lsb(bb);
    //black king
    bb = pieces_bb[BLACK][K];
    pieces[1] = 7;
    squares[1] = pop_lsb(bb);
    //white pieces
    for(int piece = Q; piece <= P; piece++) {
        bb = pieces_bb[WHITE][piece];
        while(bb) {
            square = pop_lsb(bb);
            pieces[idx]=piece+1;
            squares[idx] = square;
            idx++;
        }
    }
    for(int piece = Q; piece <= P; piece++) {
        bb = pieces_bb[BLACK][piece];
        while(bb) {
            square = pop_lsb(bb);
            pieces[idx]=piece+7;
            squares[idx] = square;
            idx++;
        }
    }

    pieces[idx] = 0;
    squares[idx] = 0;
}




int evaluateStatic() {
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

NNUEdata nn;

bool isNullBranch=false;
NNUEdata nn_stack[max_ply+1];
NNUEdata* stack[3];
template<>
int evaluate<true>()
{
    nnue_input(nnue_pieces, nnue_squares);
    int side = board.colorToMove == WHITE ? BLACK : WHITE; // the NNUE side is 0 if white to move, while our WHITE is equal to 1

    //reset computed accumulation of current ply
    nn_stack[ply].accumulator.computedAccumulation = 0;
    //fill the stack with current ply and previous 2 plies' networks
    stack[0] = &nn_stack[ply];
    stack[1] = (ply > 0) ? &nn_stack[ply - 1] : 0;
    stack[2] = (ply > 1) ? &nn_stack[ply - 2] : 0;
    return evaluate_nnue_incremental(side, nnue_pieces, nnue_squares, stack);
}

template<>
int evaluate<false>() 
{
    nnue_input(nnue_pieces, nnue_squares);
    int side = board.colorToMove == WHITE ? BLACK : WHITE; // the NNUE side is 0 if white to move, while our WHITE is equal to 1

    return evaluate_nnue(side, nnue_pieces, nnue_squares, &nn_stack[ply]);
}
template int evaluate<true>();  // Explicit instantiation for true
template int evaluate<false>(); // Explicit instantiation for false



/*
int evaluate()
{
    nnue_input(nnue_pieces, nnue_squares);
    int side = board.colorToMove == WHITE ? BLACK : WHITE; // the NNUE side is 0 if white to move, whil our WHITE is equal to 1
    //return evaluate_nnue(side, nnue_pieces, nnue_squares, &nn);
    if(isNullBranch)
        return evaluate_nnue(side, nnue_pieces, nnue_squares, &nn);

    nn_stack[nn_ply].accumulator.computedAccumulation=0;
    stack[0] = &nn_stack[nn_ply];
    stack[1] = (nn_ply > 0) ? &nn_stack[nn_ply - 1] : 0;
    stack[2] = (nn_ply > 1) ? &nn_stack[nn_ply - 2] : 0;
    // int right_eval = evaluate_nnue(side, nnue_pieces, nnue_squares, &nn);
    // int wrong_eval = evaluate_nnue_incremental(side, nnue_pieces, nnue_squares, stack);
    // if (right_eval != wrong_eval) {
    //     different++;

    //     for(int i =1; i <= nn_ply; i++) {
    //         printf("%s", coordFromPosition[getSquareFrom(nn_move_stack[i])]); // board.coordFromBitboardPosition((int)getSquareFrom(move)).c_str());
    //         printf("%s ", coordFromPosition[getSquareTo(nn_move_stack[i])]);

    //     }
    //     printf("Rigth eval: %d Wrong eval: %d ply: %d \n", right_eval, wrong_eval, nn_ply);
    //     int x;
    //     board.pretty_print_bb(board.get_occupancy());
    //     cin >> x;
    // }

    return evaluate_nnue_incremental(side, nnue_pieces, nnue_squares, stack);

    // return evaluate_nnue(side, nnue_pieces, nnue_squares ,&nn);
}
*/
