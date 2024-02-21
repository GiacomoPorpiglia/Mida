#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <string>
#include "constants.h"



using namespace std;
class Board {

public:

    const int isSlidingPiece[6] = {
        0, 1, 1, 1, 0, 0
    };

    Board(string fen);

    uint64_t A_FILE = 72340172838076673;
    uint64_t H_FILE = 9259542123273814144;
    uint64_t AB_FILE = 217020518514230019;
    uint64_t GH_FILE = 13889313184910721216;

    const int KING_OFFSETS[8] = {7, 8, 9, 1, -7, -8, -9, -1};
    const uint64_t NOT_FILES_KING[8] = {H_FILE, 0, A_FILE, A_FILE, A_FILE, 0, H_FILE, H_FILE};

    const int KNIGHT_OFFSETS[8] = {6, 15, 17, 10, -6, -15, -17, -10};
    uint64_t NOT_FILES_KNIGHT[8] = {GH_FILE, H_FILE, A_FILE, AB_FILE, AB_FILE, A_FILE, H_FILE, GH_FILE};

    CH allPieces[64] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    uint16_t boardSpecs = 0; // indices:  enPassant=0, whiteCastleKing=1, whiteCcastleQueen=2, blackCastleKing=3, blackCcastleQueen=4 values: 0 or 1 for everyone except en passant. en passant values: -1 if not allowed, square index if ep-capture allowed on that square

    //value of white and black pieces on the board, will be updated each move to make the piece count a lot faster
    int whitePiecesValue=0;
    int blackPiecesValue=0;


    uint64_t colorToMoveOccupiedBB;
    uint64_t opponentOccupiedBB;

    uint64_t occupancy;
    uint64_t attackedSquares;
    uint64_t kingPseudoLegalMoves;
    uint64_t kingMoves;

    uint64_t checkers_bb = 0ULL;

    uint64_t inCheckMask;
    uint64_t pushMask;
    uint64_t captureMask;
    int isInCheckByPawn;

    int colorToMove; // 1 if white to move, 0 if black to move
    bool isInCheck;
    int kingPos;

    void pretty_print_bb(uint64_t bb);
    void calculateMoves(int side_to_move, movesList *moveList);
    void calculateAttackedSquares(uint64_t opponentBitboards[6]);
    void reset();
    void loadFenPosition(string fen);
    bool inCheck();
    void getTotalAttackedSquares(uint64_t occupancy);

    int getWhitePieceTypeOnSquare(int square);
    int getBlackPieceTypeOnSquare(int square);
    int getPieceTypeOnSquare(int square);

    uint64_t whitePawnsAttacks(uint64_t pawn_bb);
    uint64_t blackPawnsAttacks(uint64_t pawn_bb);
    uint64_t pawnsAttacks(uint64_t pawn_bb, int side);
    uint64_t get_occupancy();
    uint64_t get_white_occupancy();
    uint64_t get_black_occupancy();

    uint64_t attackersForSide(int color, int sq, uint64_t occupancy);
    uint64_t allAttackers(int sq, uint64_t occupancy);

private:

    //bitboard of pinned pieces of the color to move
    uint64_t pinnedPiecesBB = 0ULL;

    uint64_t squaresToTraverseForWhiteCastleKing = 96;
    uint64_t squaresToTraverseForWhiteCastleQueen = 14;

    uint64_t squaresToTraverseForBlackCastleKing = 6917529027641081856;
    uint64_t squaresToTraverseForBlackCastleQueen = 1008806316530991104;

    //array containing the masks for pinned pieces indexed by square. it doesn't need to be reset every time we have a new position, because we are only resetting pinnedPiecesBB, which has hot bits of the pinned pieces positions.
    //this way we won't risk accessing old masks, since we will only access masks of pieces pinned in the CURRENT position (the positions of these pieces are stored in pinnedPiecesBB)
    uint64_t pinnedPiecesMask[64] = {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL};

    uint64_t whiteCastleKingPath = 96;
    uint64_t whiteCastleQueenPath = 14;
    uint64_t blackCastleKingPath = 6917529027641081856;
    uint64_t blackCastleQueenPath = 1008806316530991104;

    int positionFromCoord(string coord);
    uint64_t whitePawnPushMap(int square, uint64_t occupancy);
    uint64_t blackPawnPushMap(int square, uint64_t occupancy);
    void findCheckers(uint64_t opponentBitboards[6], uint64_t occupancy);

    uint64_t plus1(int square, int max = 8);
    uint64_t plus7(int square, int max = 8);
    uint64_t plus8(int square, int max = 8);
    uint64_t plus9(int square, int max = 8);
    uint64_t minus1(int square, int max = 8);
    uint64_t minus7(int square, int max = 8);
    uint64_t minus8(int square, int max = 8);
    uint64_t minus9(int square, int max = 8);

    uint64_t pushMaskFromPieceToKing(int kingPos, int checkerPos, int pieceType);

    void calculatePinnedPieces(uint64_t opponentBitboards[6], uint64_t colorToMoveOccupiedBB, uint64_t opponentOccupiedBB);
    bool isEnPassantPinned(int enPassantCapturePos, int piecePos, uint64_t opponentBitboards[6]);
    void addMoves(int squareFrom, uint64_t bb, int pieceType, movesList *moveList);
    void calculateLegalMoves(uint64_t colorToMoveBitboards[6], uint64_t opponentBitboards[6], int isInCheckByPawn, movesList *moveList);
    bool inCheckWithOccupancies(uint64_t whiteOccupancy, uint64_t blackOccupancy);
};

inline uint64_t Board::get_occupancy()
{
    return pieces_bb[WHITE][K] | pieces_bb[WHITE][Q] | pieces_bb[WHITE][R] | pieces_bb[WHITE][B] | pieces_bb[WHITE][N] | pieces_bb[WHITE][P] | pieces_bb[BLACK][K] | pieces_bb[BLACK][Q] | pieces_bb[BLACK][R] | pieces_bb[BLACK][B] | pieces_bb[BLACK][N] | pieces_bb[BLACK][P];
}

inline uint64_t Board::get_white_occupancy()
{
    return pieces_bb[WHITE][K] | pieces_bb[WHITE][Q] | pieces_bb[WHITE][R] | pieces_bb[WHITE][B] | pieces_bb[WHITE][N] | pieces_bb[WHITE][P];
}
inline uint64_t Board::get_black_occupancy()
{
    return pieces_bb[BLACK][K] | pieces_bb[BLACK][Q] | pieces_bb[BLACK][R] | pieces_bb[BLACK][B] | pieces_bb[BLACK][N] | pieces_bb[BLACK][P];
}

inline uint64_t Board::allAttackers(int sq, uint64_t occupancy)
{
    return attackersForSide(WHITE, sq, occupancy) | attackersForSide(BLACK, sq, occupancy);
}

inline uint64_t Board::whitePawnsAttacks(uint64_t pawn_bb)
{
    return (((pawn_bb << 7) & ~H_FILE) | ((pawn_bb << 9) & ~A_FILE));
}
inline uint64_t Board::blackPawnsAttacks(uint64_t pawn_bb)
{
    return (((pawn_bb >> 7) & ~A_FILE) | ((pawn_bb >> 9) & ~H_FILE));
}
inline uint64_t Board::pawnsAttacks(uint64_t pawn_bb, int side)
{
    // white
    return (side == WHITE ? whitePawnsAttacks(pawn_bb) : blackPawnsAttacks(pawn_bb));
}

inline int Board::getPieceTypeOnSquare(int square) {
    int whiteType = getWhitePieceTypeOnSquare(square);
    int blackType = getBlackPieceTypeOnSquare(square);
    return (whiteType > blackType) ? whiteType : blackType;
}

#endif