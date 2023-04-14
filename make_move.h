#ifndef MAKE_MOVE_H
#define MAKE_MOVE_H


#include "constants.h"
#include "game_constants.h"
#include "bb_helpers.h"
#include "board_declaration.h"
#include "stdint.h"
#include "hashing.h"

int squareFrom;
int squareTo;
int newPieceType;
int pieceType;
/**
 * function to play a move and update the board accordingl, also updating the hash key
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @param capturedPieceType int (-1...5) to specify if a piece is captured and which type (-1 if not a capture)
 */
static inline void playMove(MOVE move, int capturedPieceType)
{
    squareFrom = getSquareFrom(move);
    squareTo = getSquareTo(move);
    newPieceType = getNewPieceType(move);

    board.allPieces[squareFrom] = -1;
    board.allPieces[squareTo] = newPieceType;

    // increment the game ply counter
    plyGameCounter++;

    if (board.colorToMove == 1)
    {

        pieceType = board.getWhitePieceTypeOnSquare(squareFrom); // board.whitePieces[squareFrom];

        // if the move isn't a capture or a pawn move, update the fifty move rule table
        if ((pieceType != P) && (capturedPieceType == -1))
            fiftyMoveRuleTable[plyGameCounter] = fiftyMoveRuleTable[plyGameCounter - 1] + 1;
        else
            fiftyMoveRuleTable[plyGameCounter] = 0;

        // update piece value in case of capture
        if (capturedPieceType != -1)
            board.blackPiecesValue -= pieceValues[capturedPieceType];
        // update piece value in case of promotion
        board.whitePiecesValue += pieceValues[newPieceType] - pieceValues[pieceType];
        // still missing particular case of en passant(not considered as capture in this utilization, we will cover it in en passant section)

        pop_bit(board.whiteBitboards[pieceType], squareFrom);
        set_bit(board.whiteBitboards[newPieceType], squareTo);

        // en passant
        if (pieceType == P && squareTo == getEnPassantSquare(board.boardSpecs))
        {

            // update hash_key
            hash_key ^= piece_keys[1][P][squareFrom];
            hash_key ^= piece_keys[1][P][squareTo];
            hash_key ^= piece_keys[0][P][squareTo - 8];

            // update piece values in case of en passant
            board.blackPiecesValue -= pieceValues[P];

            pop_bit(board.blackBitboards[P], squareTo - 8);
            board.allPieces[squareTo - 8] = -1;
        }
        // castle
        else if (pieceType == K && abs(squareTo - squareFrom) == 2)
        {

            // update castling rights in hash key
            hash_key ^= castle_keys[generate_castle_key_index()];

            // disable white's castling rights
            board.boardSpecs ^= 3;
            // pop_bit(board.boardSpecs, 0);
            // pop_bit(board.boardSpecs, 1);

            hash_key ^= castle_keys[generate_castle_key_index()];

            // kingside
            if (squareTo - squareFrom == 2)
            {

                // update hash key
                hash_key ^= piece_keys[1][K][4];
                hash_key ^= piece_keys[1][K][6];
                hash_key ^= piece_keys[1][R][7];
                hash_key ^= piece_keys[1][R][5];

                pop_bit(board.whiteBitboards[R], 7);
                set_bit(board.whiteBitboards[R], 5);

                board.allPieces[7] = -1;
                board.allPieces[5] = 2;
            }
            // queenside
            else
            {

                // update hash key
                hash_key ^= piece_keys[1][K][4];
                hash_key ^= piece_keys[1][K][2];
                hash_key ^= piece_keys[1][R][0];
                hash_key ^= piece_keys[1][R][3];

                pop_bit(board.whiteBitboards[R], 0);
                set_bit(board.whiteBitboards[R], 3);

                board.allPieces[0] = -1;
                board.allPieces[3] = 2;
            }
        }
        // if not castle and not en passant, just update the hash key
        else
        {
            // update hash key
            hash_key ^= piece_keys[1][pieceType][squareFrom];
            hash_key ^= piece_keys[1][newPieceType][squareTo];
        }
        // if move is a capture, update opponent bitboards and hash key
        if (capturedPieceType != -1)
        {
            // update hash key
            hash_key ^= piece_keys[0][capturedPieceType][squareTo];
            // board.blackPieces[squareTo]=-1;
            pop_bit(board.blackBitboards[capturedPieceType], squareTo);
        }

        if (pieceType == K)
        {
            // update castling rights
            hash_key ^= castle_keys[generate_castle_key_index()];
            pop_bit(board.boardSpecs, 0);
            pop_bit(board.boardSpecs, 1);
            hash_key ^= castle_keys[generate_castle_key_index()];
        }
        if (pieceType == R)
        {
            if (squareFrom == 0)
            {
                hash_key ^= castle_keys[generate_castle_key_index()];
                pop_bit(board.boardSpecs, 1);
                hash_key ^= castle_keys[generate_castle_key_index()];
            }
            else if (squareFrom == 7)
            {
                hash_key ^= castle_keys[generate_castle_key_index()];
                pop_bit(board.boardSpecs, 0);
                hash_key ^= castle_keys[generate_castle_key_index()];
            }
        }

        if (getEnPassantSquare(board.boardSpecs) != -1)
            hash_key ^= en_passant_keys[getEnPassantSquare(board.boardSpecs)];

        unsetEnPassantSquare(board.boardSpecs);
        if (pieceType == 5 && squareTo - squareFrom == 16)
        {
            hash_key ^= en_passant_keys[squareTo - 8];
            setEnPassantSquare(squareTo - 8, board.boardSpecs);
        }
    }
    // black
    else
    {
        pieceType = board.getBlackPieceTypeOnSquare(squareFrom); // board.blackPieces[squareFrom];

        // if the move isn't a capture or a pawn move, update the fifty move rule table
        if ((pieceType != P) && (capturedPieceType == -1))
            fiftyMoveRuleTable[plyGameCounter] = fiftyMoveRuleTable[plyGameCounter - 1] + 1;
        else
            fiftyMoveRuleTable[plyGameCounter] = 0;

        // update piece value in case of capture
        if (capturedPieceType != -1)
            board.whitePiecesValue -= pieceValues[capturedPieceType];
        // update piece value in case of promotion
        board.blackPiecesValue += pieceValues[newPieceType] - pieceValues[pieceType];
        // still missing particular case of en passant(not considered as capture in this utilization, we will cover it in en passant section)

        pop_bit(board.blackBitboards[pieceType], squareFrom);
        set_bit(board.blackBitboards[newPieceType], squareTo);

        // en passant
        if (pieceType == P && squareTo == getEnPassantSquare(board.boardSpecs))
        {

            // update hash_key
            hash_key ^= piece_keys[0][P][squareFrom];
            hash_key ^= piece_keys[0][P][squareTo];
            hash_key ^= piece_keys[1][P][squareTo + 8];

            // update piece values in case of en passant
            board.whitePiecesValue -= pieceValues[P];

            pop_bit(board.whiteBitboards[P], squareTo + 8);

            board.allPieces[squareTo + 8] = -1;
        }
        // castle
        else if (pieceType == K && abs(squareTo - squareFrom) == 2)
        {

            // update castling rights in hash key
            hash_key ^= castle_keys[generate_castle_key_index()];

            pop_bit(board.boardSpecs, 2);
            pop_bit(board.boardSpecs, 3);

            hash_key ^= castle_keys[generate_castle_key_index()];

            // kingside
            if (squareTo - squareFrom == 2)
            {

                // update hash key
                hash_key ^= piece_keys[0][K][60];
                hash_key ^= piece_keys[0][K][62];
                hash_key ^= piece_keys[0][R][63];
                hash_key ^= piece_keys[0][R][61];

                pop_bit(board.blackBitboards[R], 63);
                set_bit(board.blackBitboards[R], 61);

                board.allPieces[63] = -1;
                board.allPieces[61] = 2;
            }
            // queenside
            else
            {

                // update hash key
                hash_key ^= piece_keys[0][K][60];
                hash_key ^= piece_keys[0][K][58];
                hash_key ^= piece_keys[0][R][56];
                hash_key ^= piece_keys[0][R][59];

                pop_bit(board.blackBitboards[R], 56);
                set_bit(board.blackBitboards[R], 59);

                board.allPieces[56] = -1;
                board.allPieces[59] = 2;
            }
        }
        // if it's not castle and not en passant, just update the hash key
        else
        {

            // update hash key
            hash_key ^= piece_keys[0][pieceType][squareFrom];
            hash_key ^= piece_keys[0][newPieceType][squareTo];
        }
        if (capturedPieceType != -1)
        {

            hash_key ^= piece_keys[1][capturedPieceType][squareTo];

            // board.whitePieces[squareTo]=-1;
            pop_bit(board.whiteBitboards[capturedPieceType], squareTo);
        }

        if (pieceType == K)
        {
            hash_key ^= castle_keys[generate_castle_key_index()];
            pop_bit(board.boardSpecs, 2);
            pop_bit(board.boardSpecs, 3);
            hash_key ^= castle_keys[generate_castle_key_index()];
        }
        if (pieceType == R)
        {
            if (squareFrom == 56)
            {
                hash_key ^= castle_keys[generate_castle_key_index()];
                pop_bit(board.boardSpecs, 3);
                hash_key ^= castle_keys[generate_castle_key_index()];
            }
            else if (squareFrom == 63)
            {
                hash_key ^= castle_keys[generate_castle_key_index()];
                pop_bit(board.boardSpecs, 2);
                hash_key ^= castle_keys[generate_castle_key_index()];
            }
        }

        if (getEnPassantSquare(board.boardSpecs) != -1)
            hash_key ^= en_passant_keys[getEnPassantSquare(board.boardSpecs)];

        unsetEnPassantSquare(board.boardSpecs);
        if (pieceType == P && squareFrom - squareTo == 16)
        {
            hash_key ^= en_passant_keys[squareTo + 8];
            setEnPassantSquare(squareTo + 8, board.boardSpecs);
        }
    }
    hash_key ^= side_key;
    board.colorToMove = (board.colorToMove + 1) % 2;
}

/**
 * function to unplay a move and update the board accordingly, also updting the hash key
 * @param move move in 16 bit format(6 bits encode the square from, 6 bits encode the square to, and 4 encode the new piece type)
 * @param oldPieceType int (0...5) to cover the particular case of promotion, in which the peice type is different from the original one, which was a pawn
 * @param oldSpecs vector<char>, that in this implementation is the format in which castling rights and en passant availbillity are stored. It is a copy of these specs before the move was played, in order to restore the appropriately
 * @param capturedPieceType int (-1...5) to specify if a piece is captured and which type (-1 if not a capture)
 */
static inline void unplayMove(MOVE move, int oldPieceType, uint16_t oldSpecs, int capturedPieceType)
{

    board.colorToMove = (board.colorToMove + 1) % 2;

    squareFrom = getSquareFrom(move);
    squareTo = getSquareTo(move);
    newPieceType = getNewPieceType(move);

    board.allPieces[squareFrom] = oldPieceType;
    board.allPieces[squareTo] = capturedPieceType;

    // reset fifty move rule value of the move to 0
    fiftyMoveRuleTable[plyGameCounter] = 0;

    // decrement ply game counter
    plyGameCounter--;

    if (board.colorToMove == 1)
    {

        // update piece value in case of promotion
        board.whitePiecesValue -= pieceValues[newPieceType] - pieceValues[oldPieceType];
        // still missing particular case of en passant(not considered as capture in this part, but in en passant move section)
        pop_bit(board.whiteBitboards[newPieceType], squareTo);
        set_bit(board.whiteBitboards[oldPieceType], squareFrom);

        // en passant
        if (oldPieceType == P && squareTo == getEnPassantSquare(oldSpecs))
        {

            // piece value update for en passant
            board.blackPiecesValue += pieceValues[P];

            set_bit(board.blackBitboards[P], squareTo - 8);

            board.allPieces[squareTo - 8] = 5;
        }
        // castle
        else if (oldPieceType == K && abs(squareTo - squareFrom) == 2)
        {
            // kingside
            if (squareTo - squareFrom == 2)
            {
                pop_bit(board.whiteBitboards[R], 5);
                set_bit(board.whiteBitboards[R], 7);

                board.allPieces[7] = 2;
                board.allPieces[5] = -1;
            }
            // queenside
            else
            {
                pop_bit(board.whiteBitboards[R], 3);
                set_bit(board.whiteBitboards[R], 0);

                board.allPieces[0] = 2;
                board.allPieces[3] = -1;
            }
        }

        if (capturedPieceType != -1)
        {
            set_bit(board.blackBitboards[capturedPieceType], squareTo);
            // update piece values in case of capture
            board.blackPiecesValue += pieceValues[capturedPieceType];
        }
    }
    // black
    else
    {

        // update piece value in case of promotion
        board.blackPiecesValue -= pieceValues[newPieceType] - pieceValues[oldPieceType];
        // still missing particular case of en passant(not considered as capture in this utilization, we will cover it in en passant section)

        pop_bit(board.blackBitboards[newPieceType], squareTo);
        set_bit(board.blackBitboards[oldPieceType], squareFrom);

        // en passant
        if (oldPieceType == P && squareTo == getEnPassantSquare(oldSpecs))
        {

            // piece value update for en passant
            board.whitePiecesValue += pieceValues[P];

            set_bit(board.whiteBitboards[P], (squareTo + 8));

            board.allPieces[squareTo + 8] = 5;
        }
        // castle
        else if (oldPieceType == K && abs(squareTo - squareFrom) == 2)
        {
            // kingside
            if (squareTo - squareFrom == 2)
            {
                pop_bit(board.blackBitboards[2], 61);
                set_bit(board.blackBitboards[2], 63);

                board.allPieces[63] = 2;
                board.allPieces[61] = -1;
            }
            // queenside
            else
            {
                pop_bit(board.blackBitboards[R], 59);
                set_bit(board.blackBitboards[R], 56);

                board.allPieces[56] = 2;
                board.allPieces[59] = -1;
            }
        }
        if (capturedPieceType != -1)
        {
            set_bit(board.whiteBitboards[capturedPieceType], squareTo);
            // update piece value in case of capture
            board.whitePiecesValue += pieceValues[capturedPieceType];
            // board.whitePieces[squareTo] = capturedPieceType;
        }
    }

    board.boardSpecs = oldSpecs;
}

// make null move(basically skipping a turn without any other change to the board, used in null move pruning)
void makeNullMove()
{
    board.colorToMove = (board.colorToMove + 1) % 2;
    hash_key ^= side_key;
    // reset en passant in hash key
    if (getEnPassantSquare(board.boardSpecs) != -1)
        hash_key ^= en_passant_keys[getEnPassantSquare(board.boardSpecs)];
}

// unmake null move(used in null move pruning)
void unmakeNullMove(uint16_t copySpecs)
{
    board.colorToMove = (board.colorToMove + 1) % 2;
    hash_key ^= side_key;
    if (getEnPassantSquare(copySpecs) != -1)
        hash_key ^= en_passant_keys[getEnPassantSquare(copySpecs)];
}

#endif