#include "Board.h"
#include "../constants.h"
#include "../evaluate.h"
#include "../magic_bitboards.h"
#include "../bitboard.h"
#include "../game_constants.h"


// using namespace std;
Board::Board (string fen) {
    loadFenPosition(fen);
}

void Board::reset() {

    repetition_index = 0;

    plyGameCounter = 0;
    resetFiftyMoveTable();

    // reset bitboards array
    for (int i = 0; i < 6; i++) {
        pieces_bb[WHITE][i] = 0ULL;
        pieces_bb[BLACK][i] = 0ULL;
    }

    //reset all pieces array
    for(int i = 0; i < 64; i++) 
        allPieces[i] = -1;

    whitePiecesValue = 0;
    blackPiecesValue = 0;

    boardSpecs = (1<<15);
    isInCheck = false;
}

void Board::loadFenPosition(string fen) {

    reset();
    vector<string> splitted = splitString(fen, (char)' ');

    int row = 7;
    int col = 0;
    int idx = 0;
    for(char ch : splitted[0]) {

        if (ch == '/') {
            row --;
            col = 0;
            idx = row*8 + col;
        } 
        else if (!isdigit(ch)) {
            idx = row*8 + col;
            if (ch == 'P') {  
                set_bit(pieces_bb[WHITE][5], idx);
                allPieces[idx] = 5;
                whitePiecesValue += pieceValues[5];
            }
            else if (ch == 'p') {
                set_bit(pieces_bb[BLACK][5], idx);
                allPieces[idx] = 5;
                blackPiecesValue += pieceValues[5];
            }
            else if (ch == 'N') {
                set_bit(pieces_bb[WHITE][4], idx);
                allPieces[idx] = 4;
                whitePiecesValue += pieceValues[4];
            }
            else if (ch == 'n') {
                set_bit(pieces_bb[BLACK][4], idx);
                allPieces[idx] = 4;
                blackPiecesValue += pieceValues[4];
            }
            else if (ch == 'B') {
                set_bit(pieces_bb[WHITE][3], idx);
                allPieces[idx] = 3;
                whitePiecesValue += pieceValues[3];
            }
            else if (ch == 'b') {
                set_bit(pieces_bb[BLACK][3], idx);
                allPieces[idx] = 3;
                blackPiecesValue += pieceValues[3];
            }
            else if (ch == 'R') {
                set_bit(pieces_bb[WHITE][2], idx);
                allPieces[idx] = 2;
                whitePiecesValue += pieceValues[2];
            }
            else if (ch == 'r') {
                set_bit(pieces_bb[BLACK][2], idx);
                allPieces[idx] = 2;
                blackPiecesValue += pieceValues[2];
            }
            else if (ch == 'Q') {
                set_bit(pieces_bb[WHITE][1], idx);
                allPieces[idx] = 1;
                whitePiecesValue += pieceValues[1];
            }
            else if (ch == 'q') {
                set_bit(pieces_bb[BLACK][1], idx);
                allPieces[idx] = 1;
                blackPiecesValue += pieceValues[1];
            }
            else if (ch == 'K') {
                set_bit(pieces_bb[WHITE][0], idx);
                allPieces[idx] = 0;
                whitePiecesValue += pieceValues[0];
            }
            else if (ch == 'k') {
                set_bit(pieces_bb[BLACK][0], idx);
                allPieces[idx] = 0;
                blackPiecesValue += pieceValues[0];
            }
            col++;
        }
        else {
            col += int(ch)-48;
        }
    }

    if (splitted[1] == "w") 
        colorToMove = 1;
    else
        colorToMove = 0;
    
    if (splitted[2] == "-") {}
    else {
        for(char ch : splitted[2]) {
            if (ch == 'K') {
                set_bit(boardSpecs, 0);
            }
            else if (ch == 'Q')
            {
                set_bit(boardSpecs, 1);
            }
            else if (ch == 'k') {
                set_bit(boardSpecs, 2);
            }
            else if (ch == 'q') {
                set_bit(boardSpecs, 3);
            }
        }
    }

    if (splitted[3] != "-") {
        setEnPassantSquare(positionFromCoord(splitted[3]), boardSpecs);
    } else unsetEnPassantSquare(boardSpecs);

}

int Board::positionFromCoord(string coord) {
    return ((int)coord[1] - 48 - 1) * 8 + (int)coord[0] - 97;
}


void Board::pretty_print_bb(uint64_t bb) {
    for (int r=7; r>=0; r--) {
        for (int c=0; c < 8; c++) {
            int idx = r*8 + c;
            if(get_bit(bb, idx))
                cout << "X ";
            else cout << "- ";
        }
        cout << endl;
    }    

}

//EAST RAY
uint64_t Board::plus1(int square, int max) {
    uint64_t bitboard = 0;
    int squareCount = 0;
    for(int i = square; i < 64; i++) {
        if (i!=square) {
            if (squareCount == max) {
                return bitboard;
            }
            set_bit(bitboard, i);
            squareCount++;
        }
        if (!((i+1)%8)) {
            return bitboard;
        }
    } 
    return bitboard;
}

//NORTHWEST RAY
uint64_t Board::plus7(int square, int max) {
    uint64_t bitboard = 0;
    int squareCount = 0;
    for(int i = square; i < 64; i+=7) {
        if (i!=square) {
            if (squareCount == max) {
                return bitboard;
            }
            set_bit(bitboard, i);
            squareCount++;
        }
        if (!(i%8)) {
            return bitboard;
        }
    } 
    return bitboard;
}

//NORTH RAY
uint64_t Board::plus8(int square, int max) {
    uint64_t bitboard = 0;
    int squareCount = 0;
    for(int i = square; i < 64; i+=8) {
        if (i!=square) {
            if (squareCount == max) {
                return bitboard;
            }
            set_bit(bitboard, i);
            squareCount++;
        }
    } 
    return bitboard;
}

//NORTHEAST RAY
uint64_t Board::plus9(int square, int max) {
    uint64_t bitboard = 0;
    int squareCount = 0;
    for(int i = square; i < 64; i+=9) {
        if (i!=square) {
            if (squareCount == max) {
                return bitboard;
            }
            set_bit(bitboard, i);
            squareCount++;
        }
        if (!((i+1)%8)) {
            return bitboard;
        }
    } 
    return bitboard;
}
//WEST RAY
uint64_t Board::minus1(int square, int max) {
    uint64_t bitboard = 0;
    int squareCount = 0;
    for(int i = square; i >=0; i-=1) {
        if (i!=square) {
            if (squareCount == max) {
                return bitboard;
            }
            set_bit(bitboard, i);
            squareCount++;
        }
        if (!(i%8)) {
            return bitboard;
        }
    } 
    return bitboard;
}

//SOUTHWEST RAY
uint64_t Board::minus7(int square, int max) {
    uint64_t bitboard = 0;
    int squareCount = 0;
    for(int i = square; i >=0; i-=7) {
        if (i!=square) {
            if (squareCount == max) {
                return bitboard;
            }
            set_bit(bitboard, i);
            squareCount++;
        }
        if (!((i+1)%8)) {
            return bitboard;
        }
    } 
    return bitboard;
}

//SOUTH RAY
uint64_t Board::minus8(int square, int max) {
    uint64_t bitboard = 0;
    int squareCount = 0;
    for(int i = square; i >=0; i-=8) {
        if (i!=square) {
            if (squareCount == max) {
                return bitboard;
            }
            set_bit(bitboard, i);
            squareCount++;
        }
    } 
    return bitboard;
}

//SOUTHEAST RAY
uint64_t Board::minus9(int square, int max) {
    uint64_t bitboard = 0;
    int squareCount = 0;
    for(int i = square; i >=0; i-=9) {
        if (i!=square) {
            if (squareCount == max) {
                return bitboard;
            }
            set_bit(bitboard, i);
            squareCount++;
        }
        if (!(i%8)) {
            return bitboard;
        }
    } 
    return bitboard;
}




uint64_t Board::knightMap(int square) {
    return knight_attacks[square];
}

uint64_t Board::whitePawnPushMap(int square, uint64_t occupancy) {
    uint64_t result = 0ULL;
    set_bit(result, square+8);
    result &= ~occupancy;
    //if pawn can move one square and starts from 2nd rank, then it can make the 2 square push
    if (result && get_bit(rank_masks[8], square)) {
        set_bit(result, square+16);
        result &= ~occupancy;
    }
    return result;
}

uint64_t Board::whitePawnAttackMap(int square) {
    return white_pawn_attacks[square];
}

uint64_t Board::blackPawnPushMap(int square, uint64_t occupnacy) {
    uint64_t result = 0ULL;
    set_bit(result, square-8);
    result &= ~occupnacy;
    // if pawn can move one square and starts from 7th rank, then it can make the 2 square push
    if (result && get_bit(rank_masks[48], square)) {
        set_bit(result, square-16);
        result &= ~occupnacy;
    }
    return result;
}

uint64_t Board::blackPawnAttackMap(int square) {
    return black_pawn_attacks[square];
}

uint64_t Board::kingMap(int square) {
    return king_attacks[square];
}

uint64_t Board::pushMaskFromPieceToKing(int kingPos, int checkerPos, int pieceType) {
    uint64_t mask = 0ULL;
    int dist;

    int kingRow = kingPos/8;
    int kingCol = kingPos%8;
    int checkerRow = checkerPos/8;
    int checkerCol = checkerPos%8;

    if (kingPos > checkerPos) {
        //on the same row ===> east ray from king
        if (kingRow == checkerRow && (pieceType==Q || pieceType==R)) {
            dist = kingPos-checkerPos;
            mask = minus1(kingPos, dist);
        }
        // on the same file =====> south ray from king
        else if (!((kingPos-checkerPos)%8) && (pieceType==Q || pieceType==R)) {
            dist = kingRow - checkerRow;
            mask = minus8(kingPos, dist);
        }
        //on upwards diagonal =====> south-east ray from king
        else if (!((kingPos-checkerPos)%9) and (pieceType==Q || pieceType==B)) {
            if ((kingPos-checkerPos)/9 == (kingCol-checkerCol)) {
                dist = kingRow - checkerRow;
                mask = minus9(kingPos, dist);
            }
        }
        //on downwards diagonal =====> south-west ray from king
        else if (!((kingPos-checkerPos)%7) && (pieceType==Q || pieceType == B)) {
            if ((kingPos-checkerPos)/7 == (checkerCol - kingCol)) {
                dist = kingRow - checkerRow;
                mask = minus7(kingPos, dist);
            }
        }
    }
    else {
        //on the same row ====> west ray from king
        if ((kingRow == checkerRow) && (pieceType == Q | pieceType == R))
        {
            dist = checkerPos-kingPos;
            mask = plus1(kingPos, dist);
        }
        //on the same file =====> north ray from king
        else if (!((checkerPos-kingPos)%8) && (pieceType==Q || pieceType==R)) {
            dist = checkerRow - kingRow;
            mask = plus8(kingPos, dist);
        }
        //on upwards diagonal =====> north-west ray from king
        else if (!((checkerPos-kingPos)%9) && (pieceType==Q || pieceType==B)) {
            if((checkerPos-kingPos)/9 == (checkerCol - kingCol)) {
                dist = checkerRow - kingRow;
                mask = plus9(kingPos, dist);     
            }
        }
        //on downwards diagonal =====> north-east ray from king
        else if (!((checkerPos-kingPos)%7) && (pieceType==Q || pieceType==B)) {
            if((checkerPos-kingPos)/7 == (kingCol - checkerCol)) {
                dist = checkerRow - kingRow;
                mask = plus7(kingPos, dist);
            }
        }
    }

    return mask;
}


void Board::findCheckers(uint64_t opponentBitboards[6], uint64_t occupancy) {
    uint64_t overlapMap=0;
    uint64_t overlapQueenMap=0;

    uint64_t bb_copy=0ULL;
    int square;

    uint64_t movesFromKing = 0ULL;

    for(int piece = Q; piece <= P; piece++) {
        bb_copy = opponentBitboards[piece];
        
        while(bb_copy) {
            square = pop_lsb(bb_copy);
            switch (piece)
            {

            case Q:
                movesFromKing = get_rook_attacks(kingPos, occupancy) | get_bishop_attacks(kingPos, occupancy);
                if (get_bit(movesFromKing & opponentBitboards[Q], square))
                    set_bit(checkers_bb, square);
            case R:
                movesFromKing = get_rook_attacks(kingPos, occupancy);
                if(get_bit(movesFromKing & opponentBitboards[R], square))
                    set_bit(checkers_bb, square);
                break;
            case B:
                movesFromKing = get_bishop_attacks(kingPos, occupancy);
                if (get_bit(movesFromKing & opponentBitboards[B], square))
                    set_bit(checkers_bb, square);
                break;
            case N:
                movesFromKing = knightMap(kingPos);
                if (get_bit(movesFromKing & opponentBitboards[N], square))
                    set_bit(checkers_bb, square);
                break;
            case P:
                //if white to move
                if (colorToMove)
                    movesFromKing = whitePawnAttackMap(kingPos);
                else
                    movesFromKing = blackPawnAttackMap(kingPos);

                if (get_bit(movesFromKing & opponentBitboards[P], square))
                    set_bit(checkers_bb, square);
                break;

            default:
                break;
            }
            if (count_bits(checkers_bb) > 1)
                return;
        }  
    }
}

void Board::calculateAttackedSquares(uint64_t opponentBitboards[6]) {
    attackedSquares = 0ULL;
    uint64_t colorToMoveBBWithoutKing = colorToMoveOccupiedBB;
    pop_bit(colorToMoveBBWithoutKing, kingPos);

    //consider the occupancy without the opponent king (reset the bit at the end of the function)
    pop_bit(occupancy, kingPos);

    uint64_t bb_copy = 0ULL;
    int square;

    for(int piece = K; piece <= P; piece++) {
        //black
        bb_copy = opponentBitboards[piece];

        while(bb_copy) {
            square = pop_lsb(bb_copy);
            switch (piece)
            {
            case K:
                attackedSquares |= kingMap(square);
                break;
            case Q:
                attackedSquares |= get_rook_attacks(square, occupancy) | get_bishop_attacks(square, occupancy);
                break;
            case R:
                attackedSquares |= get_rook_attacks(square, occupancy);
                break;
            case B:
                attackedSquares |= get_bishop_attacks(square, occupancy);
                break;
            case N:
                attackedSquares |= knightMap(square);
                break;
            case P:
                // pawn attacked squares

                //if white to move
                if (colorToMove)
                    attackedSquares |= ((opponentBitboards[P] >> 7) & ~A_FILE) | ((opponentBitboards[P] >> 9) & ~H_FILE);
                else 
                    attackedSquares |= ((opponentBitboards[P] << 7) & ~H_FILE) | ((opponentBitboards[P] << 9) & ~A_FILE);

            default:
                break;
            }
        }
    }

    set_bit(occupancy, kingPos);
}

void Board::calculatePinnedPieces(uint64_t opponentBitboards[6], uint64_t colorToMoveOccupiedBB, uint64_t opponentOccupiedBB) {


    uint64_t bb_copy=0ULL;
    uint64_t push_mask = 0ULL;
    uint64_t intersectionToMove = 0ULL;
    uint64_t intersectionOpponent = 0ULL;
    int square;

    int toMovePieceIntersected;
    int opponentPieceIntersected;
    int count_toMovePiecesIntersected = 0;
    int count_opponentPiecesIntersected = 0;

    //loop over sliding opponent pieces
    for(int piece = Q; piece <= B; piece++) {
        bb_copy = opponentBitboards[piece];

        while(bb_copy) {
            square = pop_lsb(bb_copy);

            push_mask = pushMaskFromPieceToKing(kingPos, square, piece);
            if(push_mask) {
                intersectionToMove = push_mask & colorToMoveOccupiedBB;
                intersectionOpponent = push_mask & opponentOccupiedBB;


                count_toMovePiecesIntersected=0;
                while (intersectionToMove) {
                    toMovePieceIntersected = pop_lsb(intersectionToMove);
                    count_toMovePiecesIntersected++;
                }

                count_opponentPiecesIntersected=0;
                while (intersectionOpponent) {
                    opponentPieceIntersected = pop_lsb(intersectionOpponent);
                    count_opponentPiecesIntersected++;
                }

                if (count_toMovePiecesIntersected == 1 && count_opponentPiecesIntersected == 1) {
                    int pinnedPiece = toMovePieceIntersected;
                    uint64_t movesMaskForPinnedPiece = push_mask;
                    set_bit(pinnedPiecesBB, pinnedPiece);
                    pinnedPiecesMask[pinnedPiece] = movesMaskForPinnedPiece;
                }
            }
        }
    }
}

bool Board::isEnPassantPinned(int enPassantCapturePos, int piecePos, uint64_t opponentBitboards[6]) {
    int kingRow = kingPos/8;
    int kingCol = kingPos%8;
    int pieceCol = piecePos%8;
    int enPassantRow = enPassantCapturePos/8;
    int enPassantCol = enPassantCapturePos%8;
    if (colorToMove && kingRow != (enPassantRow - 1)) return false;
    else if (!colorToMove && kingRow != (enPassantRow+1)) return false;

    if (kingCol < enPassantCol) {
        int startIdx = 0;
        if (pieceCol > enPassantCol) {
            startIdx = piecePos+1;
            for(int idx=kingPos+1; idx < piecePos-1; idx++) {
                if (get_bit(opponentOccupiedBB, idx) || get_bit(colorToMoveOccupiedBB, idx)) return false;
            }
        }
        else {
            startIdx = piecePos+2;
            for(int idx = kingPos+1; idx < piecePos; idx++) {
                if (get_bit(opponentOccupiedBB, idx) || get_bit(colorToMoveOccupiedBB, idx)) return false;
            }
        }
        for (int idx = startIdx; idx < (kingRow+1)*8; idx++) {
            if (get_bit(opponentOccupiedBB, idx) && (get_bit(opponentBitboards[Q], idx) || get_bit(opponentBitboards[R], idx))) return true;
            else if (get_bit(colorToMoveOccupiedBB, idx) || get_bit(opponentOccupiedBB, idx)) return false;
        }
    }
    else {
        int startIdx = 0;
        if(pieceCol > enPassantCol) {
            startIdx = piecePos-2;
            for(int idx = kingPos-1; idx>piecePos; idx--) {
                if(get_bit(opponentOccupiedBB, idx) || get_bit(colorToMoveOccupiedBB, idx)) return false;
            }
        }
        else {
            startIdx = piecePos-1;
            for(int idx=kingPos-1; idx > piecePos+1; idx--) {
                if(get_bit(opponentOccupiedBB, idx) || get_bit(colorToMoveOccupiedBB, idx)) return false;
            }
        }
        for(int idx = startIdx; idx > kingRow*8-1; idx--) {
            if(get_bit(opponentOccupiedBB, idx) && (get_bit(opponentBitboards[Q], idx) || get_bit(opponentBitboards[R], idx))) return true;
            else if (get_bit(colorToMoveOccupiedBB, idx) || get_bit(opponentOccupiedBB, idx)) return false;
        }

    }
    return false;
}

//adds to totalMoves the new moves: the squareFrom is passed, and the squaresTo are the hot bits of bb, which is the bitboard that contains the moves(squaresTo as hot bits) for that piece

//the format of the move is as follows
//uint16_t [4 bits for new piece type][6 bits for square to][6 bits for square from]
void Board::addMoves(int squareFrom, uint64_t bb, int pieceType) {
    
    int squareTo;
    
    
    while(bb) {
        squareTo = pop_lsb(bb);

        uint16_t move = squareFrom;
        move |= (squareTo) << 6;
        //if peice is a pawn and it's going to either rank 1 or 8, then it's a promotion
        if (pieceType == P && (get_rank[squareTo] == 0 || get_rank[squareTo] == 7)) {
            totalMoves.moves[totalMoves.count] = (move | (Q << 12));
            totalMoves.count++;
            totalMoves.moves[totalMoves.count] = (move | (R << 12));
            totalMoves.count++;
            totalMoves.moves[totalMoves.count] = (move | (B << 12));
            totalMoves.count++;
            totalMoves.moves[totalMoves.count] = (move | (N << 12));
            totalMoves.count++;
        }
        else
        {
            totalMoves.moves[totalMoves.count] = (move | (pieceType << 12));
            totalMoves.count++;
        }
    }

}

uint64_t Board::whitePawnsAttacks(uint64_t pawn_bb) {
    return (((pawn_bb << 7) & ~H_FILE) | ((pawn_bb << 9) & ~A_FILE));
}
uint64_t Board::blackPawnsAttacks(uint64_t pawn_bb)
{
    return (((pawn_bb >> 7) & ~A_FILE) | ((pawn_bb >> 9) & ~H_FILE));
}
uint64_t Board::pawnsAttacks(uint64_t pawn_bb, int side)
{   
    //white
    if(side == 1) return whitePawnsAttacks(pawn_bb);
    return blackPawnsAttacks(pawn_bb);
}

// for evaluation
void Board::getTotalAttackedSquares(uint64_t occupancy)
{

    attacked_squares[WHITE][1] = 0ULL;
    attacked_squares[WHITE][2] = 0ULL;
    attacked_squares[WHITE][3] = 0ULL;
    attacked_squares[WHITE][4] = 0ULL;

    attacked_squares[BLACK][1] = 0ULL;
    attacked_squares[BLACK][2] = 0ULL;
    attacked_squares[BLACK][3] = 0ULL;
    attacked_squares[BLACK][4] = 0ULL;

    attacked_squares[WHITE][ALL_PIECES] = 0ULL;
    attacked_squares[BLACK][ALL_PIECES] = 0ULL;
    uint64_t attack_bb;


    attacked_squares[WHITE][P] = ((pieces_bb[WHITE][P] << 7) & ~H_FILE);
    double_attacked[WHITE] = attacked_squares[WHITE][P] & ((pieces_bb[WHITE][P] << 9) & ~A_FILE);
    double_attacked_by_pawn[WHITE] = double_attacked[WHITE];
    attacked_squares[WHITE][P] |= ((pieces_bb[WHITE][P] << 9) & ~A_FILE);

    attacked_squares[BLACK][P] =  ((pieces_bb[BLACK][P] >> 7) & ~A_FILE);
    double_attacked[BLACK] = attacked_squares[BLACK][P] & ((pieces_bb[BLACK][P] >> 9) & ~H_FILE);
    double_attacked_by_pawn[BLACK] = double_attacked[BLACK];
    attacked_squares[BLACK][P] |= ((pieces_bb[BLACK][P] >> 9) & ~H_FILE);
    
    attacked_squares[WHITE][ALL_PIECES] |= attacked_squares[WHITE][P];
    attacked_squares[BLACK][ALL_PIECES] |= attacked_squares[BLACK][P];

    int square;
    uint64_t bb_copy = pieces_bb[WHITE][K];
    while(bb_copy) {
        attack_bb = kingMap(pop_lsb(bb_copy));
        attacked_squares[WHITE][K] = attack_bb;
        double_attacked[WHITE]|= (attacked_squares[WHITE][ALL_PIECES] & attack_bb);
        attacked_squares[WHITE][ALL_PIECES] |= attacked_squares[WHITE][K];
    }
    bb_copy = pieces_bb[BLACK][K];
    while(bb_copy) {
        attack_bb = kingMap(pop_lsb(bb_copy));
        attacked_squares[BLACK][K] = attack_bb;
        double_attacked[BLACK] |= (attacked_squares[BLACK][ALL_PIECES] & attack_bb);
        attacked_squares[BLACK][ALL_PIECES] |= attacked_squares[BLACK][K];
    }
    bb_copy = pieces_bb[WHITE][Q];
    while (bb_copy) {
        square = pop_lsb(bb_copy);
        attack_bb = get_rook_attacks(square, occupancy) | get_bishop_attacks(square, occupancy);
        attacked_squares[WHITE][Q] |= attack_bb;
        double_attacked[WHITE]|= (attacked_squares[WHITE][ALL_PIECES] & attack_bb);
        attacked_squares[WHITE][ALL_PIECES] |= attacked_squares[WHITE][Q];
    }
    bb_copy = pieces_bb[BLACK][Q];
    while (bb_copy) {
        square = pop_lsb(bb_copy);
        attack_bb = get_rook_attacks(square, occupancy) | get_bishop_attacks(square, occupancy);
        attacked_squares[BLACK][Q] |= attack_bb;
        double_attacked[BLACK] |= (attacked_squares[BLACK][ALL_PIECES] & attack_bb);
        attacked_squares[BLACK][ALL_PIECES] |= attacked_squares[BLACK][Q];
    }
    bb_copy = pieces_bb[WHITE][R];
    while (bb_copy) {
        attack_bb = get_rook_attacks(pop_lsb(bb_copy), occupancy);
        attacked_squares[WHITE][R] |= attack_bb;
        double_attacked[WHITE]|= (attacked_squares[WHITE][ALL_PIECES] & attack_bb);
        attacked_squares[WHITE][ALL_PIECES] |= attacked_squares[WHITE][R];
    }
    bb_copy = pieces_bb[BLACK][R];
    while (bb_copy) {
        attack_bb = get_rook_attacks(pop_lsb(bb_copy), occupancy);
        attacked_squares[BLACK][R] |= attack_bb;
        double_attacked[BLACK] |= (attacked_squares[BLACK][ALL_PIECES] & attack_bb);
        attacked_squares[BLACK][ALL_PIECES] |= attacked_squares[BLACK][R];
    }
    bb_copy = pieces_bb[WHITE][B];
    while (bb_copy) {
        attack_bb = get_bishop_attacks(pop_lsb(bb_copy), occupancy);
        attacked_squares[WHITE][B] |= attack_bb;
        double_attacked[WHITE]|= (attacked_squares[WHITE][ALL_PIECES] & attack_bb);
        attacked_squares[WHITE][ALL_PIECES] |= attacked_squares[WHITE][B];
    }
    bb_copy = pieces_bb[BLACK][B];
    while (bb_copy) {
        attack_bb = get_bishop_attacks(pop_lsb(bb_copy), occupancy);
        attacked_squares[BLACK][B] |= attack_bb;
        double_attacked[BLACK] |= (attacked_squares[BLACK][ALL_PIECES] & attack_bb);
        attacked_squares[BLACK][ALL_PIECES] |= attacked_squares[BLACK][B];
    }
    bb_copy = pieces_bb[WHITE][N];
    while (bb_copy) {
        attack_bb = knightMap(pop_lsb(bb_copy));
        attacked_squares[WHITE][N] |= attack_bb;
        double_attacked[WHITE]|= (attacked_squares[WHITE][ALL_PIECES] & attack_bb);
        attacked_squares[WHITE][ALL_PIECES] |= attacked_squares[WHITE][N];
    }
    bb_copy = pieces_bb[BLACK][N];
    while (bb_copy) {
        attack_bb = knightMap(pop_lsb(bb_copy));
        attacked_squares[BLACK][N] |= attack_bb;
        double_attacked[BLACK] |= (attacked_squares[BLACK][ALL_PIECES] & attack_bb);
        attacked_squares[BLACK][ALL_PIECES] |= attacked_squares[BLACK][N];
    }

}

void Board::calculateLegalMoves(uint64_t colorToMoveBitboards[6], uint64_t opponentBitboards[6], int isInCheckByPawn)
{
    uint64_t bb_copy = 0ULL;
    uint64_t tempMoves = 0ULL;
    uint64_t take_pawn_checker_ep = 0ULL;

    int square;

    for (int pieceType = Q; pieceType <= P; pieceType++) {
        bb_copy = colorToMoveBitboards[pieceType];
        
        while(bb_copy) {
            square = pop_lsb(bb_copy);
            take_pawn_checker_ep = 0ULL;

            switch (pieceType)
            {
            case Q:
                tempMoves = (get_rook_attacks(square, occupancy) | get_bishop_attacks(square, occupancy)) & ~colorToMoveOccupiedBB;
                break;
            case R:
                tempMoves = get_rook_attacks(square, occupancy) & ~colorToMoveOccupiedBB;
                break;
            case B:
                tempMoves = get_bishop_attacks(square, occupancy) & ~colorToMoveOccupiedBB;
                break;
            case N:
                tempMoves = knightMap(square) & ~colorToMoveOccupiedBB;
                break;
            case P:
            //if white to move
            if (colorToMove) {
                tempMoves = whitePawnPushMap(square, occupancy);
                uint64_t attackMap = whitePawnAttackMap(square);
                tempMoves |= attackMap & opponentOccupiedBB;
                int enPassantSquare = getEnPassantSquare(boardSpecs);
                if (enPassantSquare != -1) {
                    if (get_bit(attackMap, enPassantSquare) && !isEnPassantPinned(enPassantSquare, square, opponentBitboards)) {
                        set_bit(tempMoves, enPassantSquare);
                    }
                    if (enPassantSquare == (isInCheckByPawn + 8) && get_bit(attackMap, enPassantSquare))
                    {
                        set_bit(take_pawn_checker_ep, enPassantSquare);
                    }
                }
                
            }
            else {
                tempMoves = blackPawnPushMap(square, occupancy);
                uint64_t attackMap = blackPawnAttackMap(square);
                tempMoves |= attackMap & opponentOccupiedBB;
                int enPassantSquare = getEnPassantSquare(boardSpecs);
                if (enPassantSquare != -1) {
                    if (get_bit(attackMap, enPassantSquare) && !isEnPassantPinned(enPassantSquare, square, opponentBitboards)) {
                        set_bit(tempMoves, enPassantSquare);
                    }
                    if (enPassantSquare == (isInCheckByPawn - 8) && get_bit(attackMap, enPassantSquare)) {
                        set_bit(take_pawn_checker_ep, enPassantSquare);
                    }
                }
                
            }
            break;

            default:
                break;
            }

            tempMoves &= inCheckMask;
            tempMoves |= take_pawn_checker_ep;

            if (get_bit(pinnedPiecesBB, square))
                tempMoves &= pinnedPiecesMask[square];

            addMoves(square, tempMoves, pieceType);
        }
    }
}


int Board::getWhitePieceTypeOnSquare(int square) {
    //loop over white bitboards to find, if there is, the piece type on the specified square
    for(int pieceType = K; pieceType <= P; pieceType++) {
        if(get_bit(pieces_bb[WHITE][pieceType], square)) return pieceType;
    }
    return -1;
}

int Board::getBlackPieceTypeOnSquare(int square) {
    //loop over white bitboards to find, if there is, the piece type on the specified square
    for(int pieceType = K; pieceType <= P; pieceType++) {
        if(get_bit(pieces_bb[BLACK][pieceType], square)) return pieceType;
    }
    return -1;
}

int Board::getPieceTypeOnSquare(int square) {
    int whiteType = getWhitePieceTypeOnSquare(square);
    int blackType = getBlackPieceTypeOnSquare(square);
    return (whiteType > blackType) ? whiteType : blackType;
}


movesList Board::calculateMoves(uint64_t colorToMoveBitboards[6], uint64_t opponentBitboards[6])
{
    totalMoves.count = 0;
    // totalMoves.clear();
    checkers_bb = 0ULL;
    // checkers.clear();
    isInCheck = false;
    pinnedPiecesBB = 0ULL;
    //get color to move king position
    kingPos = bitScanForward(colorToMoveBitboards[K]);


    colorToMoveOccupiedBB = colorToMoveBitboards[K] | colorToMoveBitboards[Q] | colorToMoveBitboards[R] | colorToMoveBitboards[B] | colorToMoveBitboards[N] | colorToMoveBitboards[P];

    opponentOccupiedBB = opponentBitboards[K] | opponentBitboards[Q] | opponentBitboards[R] | opponentBitboards[B] | opponentBitboards[N] | opponentBitboards[P];

    occupancy = colorToMoveOccupiedBB | opponentOccupiedBB;

    //updates attackedSquares bitboad, with squares attacked by the opponent
    calculateAttackedSquares(opponentBitboards);
    kingMoves = kingMap(kingPos) & ~(colorToMoveOccupiedBB | attackedSquares);

    //see if the king is in check. in case there are checkers, the bb checkers_bb will contain the checkers positions as hot bits
    findCheckers(opponentBitboards, occupancy);

    inCheckMask = 0ULL;
    pushMask = 0ULL;
    captureMask = 0ULL;
    isInCheckByPawn = -1; // -1 f is not checked by pawn, if it is the value will be the pos of the pawn

    if(count_bits(checkers_bb)) {
        isInCheck = true;
    }
    //if the king is in double check, only king moves are allowed. so we can return early
    if (count_bits(checkers_bb) > 1) {
        addMoves(kingPos, kingMoves, K);
        return totalMoves;
    }
    //if only one checker
    else if (count_bits(checkers_bb)) {
        int checker = bitScanForward(checkers_bb); //checker pos
        if(get_bit(opponentBitboards[P], checker)) 
            isInCheckByPawn = checker;

        set_bit(captureMask, checker);

        //if checker is sliding piece, calculate the push mask to block the check
        if(get_bit(opponentBitboards[Q], checker))
            pushMask = pushMaskFromPieceToKing(kingPos, checker, Q);
        else if (get_bit(opponentBitboards[R], checker))
            pushMask = pushMaskFromPieceToKing(kingPos, checker, R);
        else if (get_bit(opponentBitboards[B], checker))
            pushMask = pushMaskFromPieceToKing(kingPos, checker, B);

        inCheckMask = captureMask | pushMask;
    }
    else
    {
        inCheckMask = ~inCheckMask;
    }

    calculatePinnedPieces(opponentBitboards, colorToMoveOccupiedBB, opponentOccupiedBB);

    //calculate the legal moves for every piece, considering pins and everything
    calculateLegalMoves(colorToMoveBitboards, opponentBitboards, isInCheckByPawn);

    //if king is in check, don't look for castling moves
    if (count_bits(checkers_bb)) {
        addMoves(kingPos, kingMoves, K);
        return totalMoves;
    }
    else {
        //if white to move
        if (colorToMove) {
            if (canWhiteCastleKing(boardSpecs) && get_bit(colorToMoveBitboards[R], 7) && !(get_bit(attackedSquares, 5)) && !(get_bit(attackedSquares, 6)) && !(occupancy & whiteCastleKingPath))
                set_bit(kingMoves, 6);
            if (canWhiteCastleQueen(boardSpecs) && get_bit(colorToMoveBitboards[R], 0) && !get_bit(attackedSquares, 2) && !get_bit(attackedSquares, 3) && !(occupancy & whiteCastleQueenPath))
                set_bit(kingMoves, 2);
        }
        else {
            if (canBlackCastleKing(boardSpecs) && get_bit(colorToMoveBitboards[R], 63) && !get_bit(attackedSquares, 61) && !get_bit(attackedSquares, 62) &&!(occupancy & blackCastleKingPath))
                set_bit(kingMoves, 62);
            if (canBlackCastleQueen(boardSpecs) && get_bit(colorToMoveBitboards[R], 56) && !get_bit(attackedSquares, 58) && !get_bit(attackedSquares, 59) &&!(occupancy & blackCastleQueenPath))
                set_bit(kingMoves, 58);
        }
        addMoves(kingPos, kingMoves, K);
    }
    return totalMoves;
}

movesList Board::calculateWhiteMoves() {
    return calculateMoves(pieces_bb[WHITE], pieces_bb[BLACK]);
}

movesList Board::calculateBlackMoves() {
    return calculateMoves(pieces_bb[BLACK], pieces_bb[WHITE]);
}