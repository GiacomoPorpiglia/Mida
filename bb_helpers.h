#ifndef BB_HELPERS_H
#define BB_HELPERS_H
#include <stdint.h>
#include <vector>
#include <string>
using namespace std;

#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? (bitboard ^= (1ULL << square)) : 0)
#define bitScanForward(bb) (std::__countr_zero(bb))
#define count_bits(bb) (__builtin_popcountll(bb))

#define FLIP(sq) ((sq) ^ 56)


static inline int pop_lsb(uint64_t &bb)
{
    auto index = bitScanForward(bb);
    bb &= bb - 1; // unset least significant bit
    return index;
}

static inline int getEnPassantSquare(uint16_t boardSpecs)
{
    if ((boardSpecs >> 4) < 64)
    {
        return (int)(boardSpecs >> 4);
    }
    else
        return -1;
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
 * @return squareFrom int (0...63)
 */
int getSquareFrom(MOVE move)
{
    move = move << 10;
    return move >> 10;
}
/**
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @return squareTo int (0...63)
 */
int getSquareTo(MOVE move)
{
    move = move << 4;
    return move >> 10;
}
/**
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @return new piece type int (0...5 (each value corresponds to a chess piece type))
 */
int getNewPieceType(MOVE move)
{
    return move >> 12;
}

vector <string> splitString(string str, char delim)
{
    vector<string> result;
    string buf = "";
    int i = 0;
    while (i < str.length())
    {
        if (str[i] != delim)
            buf += str[i];
        else if (buf.length() > 0)
        {
            result.push_back(buf);
            buf = "";
        }
        i++;
    }
    if (!buf.empty())
        result.push_back(buf);
    return result;
}

#endif