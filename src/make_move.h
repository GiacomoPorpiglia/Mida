#ifndef MAKE_MOVE_H
#define MAKE_MOVE_H


#include "constants.h"
#include "game_constants.h"
#include "bitboard.h"
#include "board_declaration.h"
#include "stdint.h"
#include "hashing.h"

extern int squareFrom;
extern int squareTo;
extern int newPieceType;
extern int pieceType;
/**
 * function to play a move and update the board accordingl, also updating the hash key
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @param capturedPieceType int (-1...5) to specify if a piece is captured and which type (-1 if not a capture)
 */
void playMove(MOVE move);
/**
 * function to unplay a move and update the board accordingly, also updting the hash key
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @param oldPieceType int (0...5) to cover the particular case of promotion, in which the peice type is different from the original one, which was a pawn
 * @param oldSpecs vector<char>, that in this implementation is the format in which castling rights and en passant availbillity are stored. It is a copy of these specs before the move was played, in order to restore the appropriately
 * @param capturedPieceType int (-1...5) to specify if a piece is captured and which type (-1 if not a capture)
 */
void unplayMove(MOVE move, int oldPieceType, uint16_t oldSpecs, int capturedPieceType);
// make null move(basically skipping a turn without any other change to the board, used in null move pruning)
void makeNullMove();

// unmake null move(used in null move pruning)
void unmakeNullMove(uint16_t copySpecs);
#endif