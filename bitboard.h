#ifndef BITBOARD_H
#define BITBOARD_H
#include <stdint.h>
#include "constants.h"
#include <vector>
#include <string>

#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? (bitboard ^= (1ULL << square)) : 0)
#define bitScanForward(bb) (std::__countr_zero(bb))
#define count_bits(bb) (__builtin_popcountll(bb))

#define FLIP(sq) ((sq) ^ 56)

int pop_lsb(uint64_t &bb);
int getEnPassantSquare(uint16_t boardSpecs);
void unsetEnPassantSquare(uint16_t &boardSpecs);
void setEnPassantSquare(int square, uint16_t &boardSpecs);

int canWhiteCastleKing(uint16_t boardSpecs);
int canWhiteCastleQueen(uint16_t boardSpecs);
int canBlackCastleKing(uint16_t boardSpecs);
int canBlackCastleQueen(uint16_t boardSpecs);
/**
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @return squareFrom int (0...63)
 */
int getSquareFrom(MOVE move);
/**
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @return squareTo int (0...63)
 */
int getSquareTo(MOVE move);
/**
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @return new piece type int (K...P (each value corresponds to a chess piece type))
 */
int getNewPieceType(MOVE move);
std::vector<std::string> splitString(std::string str, char delim);

#endif