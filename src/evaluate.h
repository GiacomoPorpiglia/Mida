#ifndef EVALUATE_H
#define EVALUATE_H

#include <stdint.h>
#include "constants.h"
#include "bitboard.h"
#include "nnue.h"


extern const int maxEndgameWeight;// 256
extern int endgameWeight;
extern uint64_t occupancy;// = 0ULL;

extern uint64_t attackedSquares;
extern uint64_t xRayAttackedSquares;

extern int square, relativeSquare, double_pawns;
extern uint64_t bb, virtualBishopMobility, virtualRookMobility, virtualMobility;

extern const int knightPhase;
extern const int bishopPhase;
extern const int rookPhase;
extern const int queenPhase;
extern const int totalPhase;
extern int file, score, passedRank;
extern uint64_t defended, weak, stronglyProtected;
extern uint64_t *passed_mask;
extern int Us;
extern int Them;
extern int Up;
extern uint64_t excludedFromMobility, mobilityArea, lowRanks, spaceMask, behind, blockedPawns;
extern uint64_t enemyKingRing, ourKingRing;
extern int attackingPiecesCount, valueOfAttacks;
extern int ourKingSq, enemyKingSq;
extern int bonus;

extern uint64_t passed, opposed, blocked, stoppers, lever, leverPush, doubled, neighbours, phalanx, support, backward, ourPawns, theirPawns, squaresToQueen, unsafeSquares, nonPawnEnemies, safe;

static inline int fade(const int values[2]);
static inline int fade_onfly(int mg, int eg);
static inline int calculateEndgameWeight();
static inline int distance(int sq1, int sq2);
static inline int kingProximity(int kingSq, int sq);
static inline int get_relative_square(int square);
static inline uint64_t Shift(uint64_t bb, int dir);
static inline bool bool_val(uint64_t bb);
static inline int more_than_one(uint64_t bb);
static inline bool isPassedPawn(int s);
static inline int evaluateSide();
int evaluateStatic();
template <bool useIncrementalNNUE> int evaluate();
extern NNUEdata nn;
extern NNUEdata nn_stack[max_ply+1];
extern bool isNullBranch;

extern int nnue_pieces[33];
extern int nnue_squares[33];


static inline void nnue_input(int *pieces, int *squares);

/*=========================================================\
|      arbitrary values of bonuses and penalties for       |
|     pawn structures, king safety and piece activity      |
\=========================================================*/



// bitboards for evaluation (filed in board.getTotalAttackedSquares function)
extern uint64_t attacked_squares[2][7];
extern uint64_t double_attacked_by_pawn[2];
extern uint64_t double_attacked[2];

extern const int supportPawnBanus[2];
extern const int phalanxPawnBouns[2];

extern const int isolated_pawn_penalty[2];
extern const int double_pawn_penalty[2];

extern const int passed_pawn_bonus[8][2]; // for rank
extern const int passedFile[2];

extern const int supported_passed_pawn_by_rook_bonus[2];
extern const int attacked_passed_pawn_by_rook_penalty[2];
extern const int outside_passed_pawn_bonus[2];
extern const int freeBlockSquareBonus[2];

extern const int weakQueenProtection[2];
extern const int hangingBonus[2];
extern const int threatByMinor[6][2];
extern const int threatByRook[6][2];

//bonus for rook on open/semiopen file
extern const int semiopen_file_score;
extern const int open_file_score;

//bonus for each shield pawn
extern const int king_pawn_shield_bonus[2];
//penalty if king is on pawnless flank
extern const int pawnlessFlank[2];
//penalty for each pawn on same color as bishop
extern const int bishopPawns[2];
//penalty for each enemy pawn the bishop x-rays
extern const int bishopXRayPawns[2];
//bonus if bishop is on long diagonal
extern const int bishopOnLongDiagonal[2];
//bonus for the side with the bishop pair
extern const int bishop_pair_bonus;

//bonus for outpost (knight/bishop)
extern const int outpostBonus[2];
//bonus if minor piee is behind friendly pawn
extern const int minorBehindPawnBonus[2];

//bonus if rook or bishop attacks a square on the enemy king ring
extern const int rookOnKingRing[2];
extern const int bishopOnKingRing[2];

//bonus if pawns threat enemy pieces
extern const int threatByPawnPush[2]; // 25 0
extern const int threatBySafePawn[2];
//bonus for king threats
extern const int threatByKing[2];

//stockfish
extern const int psqt[6][64][2];// bonus to each piece according to how many squares they can go to
extern const int mobilityBonus[][32][2];

// penalty based onhow many squares the king sees (meaning that it is more exposed). To calculate the squarees seen by the king, the king is seen as a queen, therefore seeing in all 8 directions
extern const int virtualMobilityPenalty[28][2];

//weight of the attacker indexed by piece type 
extern const int attackerWeight[6];
//attackWeight indexed by number of attackers
extern const int attackWeight[7][2];
#endif