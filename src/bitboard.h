#ifndef BITBOARD_H
#define BITBOARD_H
#include <stdint.h>
#include "constants.h"
#include "board_declaration.h"
#include <vector>
#include <string>

#if defined(_MSC_VER) && defined(_WIN64) // for _mm_popcnt_64
#include <intrin.h>
#endif

#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? (bitboard ^= (1ULL << square)) : 0)
#define FLIP(sq) ((sq) ^ 56)

inline int bitScanForward(uint64_t bb) {
    unsigned long idx;
    _BitScanForward64(&idx, bb);
    return (int)idx;
}

inline int count_bits(uint64_t bb) {
    #if defined(_MSC_VER) || defined(__INTEL_COMPILER)

    return (int)_mm_popcnt_u64(bb);

    #else

        return __builtin_popcountll(bb);

    #endif
}


static inline int pop_lsb(uint64_t &bb)
{
    auto index = bitScanForward(bb);
    bb &= bb - 1; // unset least significant bit
    return index;
}

static inline int getEnPassantSquare(uint16_t boardSpecs)
{
    return ((boardSpecs >> 4) < 64) ? ((int)(boardSpecs >> 4)) : -1;
}

static inline void unsetEnPassantSquare(uint16_t &boardSpecs)
{
    boardSpecs = boardSpecs << 12;
    boardSpecs = boardSpecs >> 12;
    boardSpecs |= (1 << 15);
}

static inline void setEnPassantSquare(int square, uint16_t &boardSpecs)
{
    unsetEnPassantSquare(boardSpecs);
    pop_bit(boardSpecs, 15);
    boardSpecs |= ((uint16_t)(square) << 4);
}

static inline int canWhiteCastleKing(uint16_t boardSpecs)
{
    return get_bit(boardSpecs, 0);
}

static inline int canWhiteCastleQueen(uint16_t boardSpecs)
{
    return get_bit(boardSpecs, 1);
}

static inline int canBlackCastleKing(uint16_t boardSpecs)
{
    return get_bit(boardSpecs, 2);
}
static inline int canBlackCastleQueen(uint16_t boardSpecs)
{
    return get_bit(boardSpecs, 3);
}

/**
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @return new piece type int (0...5 (each value corresponds to a chess piece type))
 */
static inline int getNewPieceType(MOVE move)
{
    return move >> 12;
}

/**
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @return squareFrom int (0...63)
 */
static inline int getSquareFrom(MOVE move)
{
    move = move << 10;
    return move >> 10;
}
/**
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @return squareTo int (0...63)
 */
static inline int getSquareTo(MOVE move)
{
    move = move << 4;
    return move >> 10;
}

static inline bool isPromotion(MOVE move)
{
    return (getNewPieceType(move) != board.allPieces[getSquareFrom(move)]);
}

static inline bool isCastle(MOVE move)
{
    int from = getSquareFrom(move);
    int to = getSquareTo(move);
    return (getNewPieceType(move) == K && abs(from - to) == 2);
}

static inline bool isCapture(MOVE move)
{
    return board.allPieces[getSquareTo(move)] != NOPIECE;
}

static inline bool isEnPassant(MOVE move)
{
    int from = getSquareFrom(move);
    int to = getSquareTo(move);
    return (board.allPieces[from] == P && board.allPieces[to] == NOPIECE && abs(from - to) != 8 && abs(from - to) != 16);
}


std::vector<std::string> splitString(std::string str, char delim);

#endif