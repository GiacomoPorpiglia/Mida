#include "make_move.h"
#include "constants.h"
#include "game_constants.h"
#include "bitboard.h"
#include "board_declaration.h"
#include "hashing.h"
#include <stdint.h>

int squareFrom = 0, squareTo = 0, newPieceType = 0, pieceType = 0;

void playMove(MOVE move)
{
    squareFrom = getSquareFrom(move);
    squareTo = getSquareTo(move);
    newPieceType = getNewPieceType(move);

    int capturedPieceType = board.allPieces[squareTo];

    board.allPieces[squareFrom] = NOPIECE;
    board.allPieces[squareTo] = newPieceType;

    // increment the game ply counter
    plyGameCounter++;

    if (board.colorToMove == WHITE)
    {

        pieceType = board.getWhitePieceTypeOnSquare(squareFrom); // board.whitePieces[squareFrom];

        // if the move isn't a capture or a pawn move, update the fifty move rule table
        if ((pieceType != P) && (capturedPieceType == NOPIECE))
            fiftyMoveRuleTable[plyGameCounter] = fiftyMoveRuleTable[plyGameCounter - 1] + 1;
        else
            fiftyMoveRuleTable[plyGameCounter] = 0;

        // update piece value in case of capture
        if (capturedPieceType != NOPIECE)
            board.blackPiecesValue -= pieceValues[capturedPieceType];
        // update piece value in case of promotion
        board.whitePiecesValue += pieceValues[newPieceType] - pieceValues[pieceType];
        // still missing particular case of en passant(not considered as capture in this utilization, we will cover it in en passant section)

        pop_bit(pieces_bb[WHITE][pieceType], squareFrom);
        set_bit(pieces_bb[WHITE][newPieceType], squareTo);

        // en passant
        if (pieceType == P && squareTo == getEnPassantSquare(board.boardSpecs))
        {
            // update hash_key
            hash_key ^= piece_keys[WHITE][P][squareFrom];
            hash_key ^= piece_keys[WHITE][P][squareTo];
            hash_key ^= piece_keys[BLACK][P][squareTo - 8];

            // update piece values in case of en passant
            board.blackPiecesValue -= pieceValues[P];

            pop_bit(pieces_bb[BLACK][P], squareTo - 8);
            board.allPieces[squareTo - 8] = NOPIECE;
        }
        // castle
        else if (isCastle(move))
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
                hash_key ^= piece_keys[WHITE][K][4];
                hash_key ^= piece_keys[WHITE][K][6];
                hash_key ^= piece_keys[WHITE][R][7];
                hash_key ^= piece_keys[WHITE][R][5];

                pop_bit(pieces_bb[WHITE][R], 7);
                set_bit(pieces_bb[WHITE][R], 5);

                board.allPieces[7] = NOPIECE;
                board.allPieces[5] = R;
            }
            // queenside
            else
            {

                // update hash key
                hash_key ^= piece_keys[WHITE][K][4];
                hash_key ^= piece_keys[WHITE][K][2];
                hash_key ^= piece_keys[WHITE][R][0];
                hash_key ^= piece_keys[WHITE][R][3];

                pop_bit(pieces_bb[WHITE][R], 0);
                set_bit(pieces_bb[WHITE][R], 3);

                board.allPieces[0] = NOPIECE;
                board.allPieces[3] = R;
            }
        }
        // if not castle and not en passant, just update the hash key
        else
        {
            // update hash key
            hash_key ^= piece_keys[WHITE][pieceType][squareFrom];
            hash_key ^= piece_keys[WHITE][newPieceType][squareTo];
        }
        // if move is a capture, update opponent bitboards and hash key
        if (capturedPieceType != NOPIECE)
        {
            // update hash key
            hash_key ^= piece_keys[BLACK][capturedPieceType][squareTo];
            // board.blackPieces[squareTo]=-1;
            pop_bit(pieces_bb[BLACK][capturedPieceType], squareTo);
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
        if (pieceType == P && ((squareTo - squareFrom) == 16))
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
        if ((pieceType != P) && (capturedPieceType == NOPIECE))
            fiftyMoveRuleTable[plyGameCounter] = fiftyMoveRuleTable[plyGameCounter - 1] + 1;
        else
            fiftyMoveRuleTable[plyGameCounter] = 0;

        // update piece value in case of capture
        if (capturedPieceType != NOPIECE)
            board.whitePiecesValue -= pieceValues[capturedPieceType];
        // update piece value in case of promotion
        board.blackPiecesValue += pieceValues[newPieceType] - pieceValues[pieceType];
        // still missing particular case of en passant(not considered as capture in this utilization, we will cover it in en passant section)

        pop_bit(pieces_bb[BLACK][pieceType], squareFrom);
        set_bit(pieces_bb[BLACK][newPieceType], squareTo);

        // en passant
        if (pieceType == P && squareTo == getEnPassantSquare(board.boardSpecs))
        {   
            // update hash_key
            hash_key ^= piece_keys[BLACK][P][squareFrom];
            hash_key ^= piece_keys[BLACK][P][squareTo];
            hash_key ^= piece_keys[WHITE][P][squareTo + 8];

            // update piece values in case of en passant
            board.whitePiecesValue -= pieceValues[P];

            pop_bit(pieces_bb[WHITE][P], squareTo + 8);

            board.allPieces[squareTo + 8] = NOPIECE;
        }
        // castle
        else if (isCastle(move))
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
                hash_key ^= piece_keys[BLACK][K][60];
                hash_key ^= piece_keys[BLACK][K][62];
                hash_key ^= piece_keys[BLACK][R][63];
                hash_key ^= piece_keys[BLACK][R][61];

                pop_bit(pieces_bb[BLACK][R], 63);
                set_bit(pieces_bb[BLACK][R], 61);

                board.allPieces[63] = NOPIECE;
                board.allPieces[61] = R;
            }
            // queenside
            else
            {

                // update hash key
                hash_key ^= piece_keys[BLACK][K][60];
                hash_key ^= piece_keys[BLACK][K][58];
                hash_key ^= piece_keys[BLACK][R][56];
                hash_key ^= piece_keys[BLACK][R][59];

                pop_bit(pieces_bb[BLACK][R], 56);
                set_bit(pieces_bb[BLACK][R], 59);

                board.allPieces[56] = NOPIECE;
                board.allPieces[59] = R;
            }
        }
        // if it's not castle and not en passant, just update the hash key
        else
        {

            // update hash key
            hash_key ^= piece_keys[BLACK][pieceType][squareFrom];
            hash_key ^= piece_keys[BLACK][newPieceType][squareTo];
        }
        if (capturedPieceType != NOPIECE)
        {

            hash_key ^= piece_keys[WHITE][capturedPieceType][squareTo];

            // board.whitePieces[squareTo]=-1;
            pop_bit(pieces_bb[WHITE][capturedPieceType], squareTo);
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

        if (getEnPassantSquare(board.boardSpecs) != NOPIECE)
            hash_key ^= en_passant_keys[getEnPassantSquare(board.boardSpecs)];

        unsetEnPassantSquare(board.boardSpecs);
        if (pieceType == P && ((squareFrom - squareTo) == 16))
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
void unplayMove(MOVE move, int oldPieceType, uint16_t oldSpecs, int capturedPieceType)
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

    if (board.colorToMove == WHITE)
    {

        // update piece value in case of promotion
        board.whitePiecesValue -= pieceValues[newPieceType] - pieceValues[oldPieceType];
        // still missing particular case of en passant(not considered as capture in this part, but in en passant move section)
        pop_bit(pieces_bb[WHITE][newPieceType], squareTo);
        set_bit(pieces_bb[WHITE][oldPieceType], squareFrom);

        // en passant
        if (oldPieceType == P && squareTo == getEnPassantSquare(oldSpecs))
        {

            // piece value update for en passant
            board.blackPiecesValue += pieceValues[P];

            set_bit(pieces_bb[BLACK][P], squareTo - 8);

            board.allPieces[squareTo - 8] = 5;
        }
        // castle
        else if (oldPieceType == K && abs(squareTo - squareFrom) == 2)
        {
            // kingside
            if (squareTo - squareFrom == 2)
            {
                pop_bit(pieces_bb[WHITE][R], 5);
                set_bit(pieces_bb[WHITE][R], 7);

                board.allPieces[7] = 2;
                board.allPieces[5] = -1;
            }
            // queenside
            else
            {
                pop_bit(pieces_bb[WHITE][R], 3);
                set_bit(pieces_bb[WHITE][R], 0);

                board.allPieces[0] = 2;
                board.allPieces[3] = -1;
            }
        }

        if (capturedPieceType != NOPIECE)
        {
            set_bit(pieces_bb[BLACK][capturedPieceType], squareTo);
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

        pop_bit(pieces_bb[BLACK][newPieceType], squareTo);
        set_bit(pieces_bb[BLACK][oldPieceType], squareFrom);

        // en passant
        if (oldPieceType == P && squareTo == getEnPassantSquare(oldSpecs))
        {

            // piece value update for en passant
            board.whitePiecesValue += pieceValues[P];

            set_bit(pieces_bb[WHITE][P], (squareTo + 8));

            board.allPieces[squareTo + 8] = P;
        }
        // castle
        else if (oldPieceType == K && abs(squareTo - squareFrom) == 2)
        {
            // kingside
            if (squareTo - squareFrom == 2)
            {
                pop_bit(pieces_bb[BLACK][2], 61);
                set_bit(pieces_bb[BLACK][2], 63);

                board.allPieces[63] = 2;
                board.allPieces[61] = -1;
            }
            // queenside
            else
            {
                pop_bit(pieces_bb[BLACK][R], 59);
                set_bit(pieces_bb[BLACK][R], 56);

                board.allPieces[56] = 2;
                board.allPieces[59] = -1;
            }
        }
        if (capturedPieceType != NOPIECE)
        {
            set_bit(pieces_bb[WHITE][capturedPieceType], squareTo);
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