#include "see.h"
#include "board_declaration.h"
#include "bitboard.h"
#include "constants.h"
#include "magic_bitboards.h"


//incomplete implementation of Satatic exchange evaluation (used only in move sorting to better sort MMV/LVA moves)
bool see(MOVE move, int threshold) {

    int to_square = getSquareTo(move);
    int from_square = getSquareFrom(move);

    int target = board.allPieces[to_square];
    int value = -threshold;
    if(target!=NOPIECE) {
        value += pieceValues[target];
    }

    if (value < 0)
    {
        return false;
    }

    int attacker = board.allPieces[from_square];

    int color_from = get_bit(board.get_white_occupancy(), from_square) ? WHITE : BLACK;

    value -= pieceValues[attacker];

    if (value >= 0)
    {
        return true;
    }

    uint64_t white_occupancy = board.get_white_occupancy();
    uint64_t black_occupancy = board.get_black_occupancy();

    uint64_t occupied = (board.get_occupancy() ^ (1ULL << from_square)) | (1ULL << to_square);
    uint64_t attackers = board.allAttackers(to_square, occupied) & occupied;

    uint64_t queens = pieces_bb[WHITE][Q] | pieces_bb[BLACK][Q];

    uint64_t bishops = pieces_bb[WHITE][B] | pieces_bb[BLACK][B] | queens;
    uint64_t rooks = pieces_bb[WHITE][R] | pieces_bb[BLACK][R] | queens;

    int st = get_bit(white_occupancy, from_square) ? BLACK : WHITE; // color of captured piece

    while (true){
        attackers &= occupied;

        uint64_t myAttackers = attackers & (st == WHITE ? white_occupancy : black_occupancy);

        if (!myAttackers){
            break;
        }

        int pt;
        for (pt = 5; pt >= 0; pt--){
            if (myAttackers & (pieces_bb[WHITE][pt] | pieces_bb[BLACK][pt])){
                break;
            }
        }

        st = !st;
        if ((value = -value - 1 - pieceValues[pt]) >= 0)
        {
            if (pt == K && (attackers & (st == WHITE ? white_occupancy : black_occupancy)))
                st = !st;
            break;
        }

        occupied ^= (1ULL << (bitScanForward(myAttackers & (pieces_bb[WHITE][pt] | pieces_bb[BLACK][pt]))));

        if (pt == P || pt == B || pt == Q)
            attackers |= get_bishop_attacks(to_square, occupied) & bishops;
        if (pt == R || pt == Q)
            attackers |= get_rook_attacks(to_square, occupied) & rooks;
    }
    return st != color_from;
}