
#include "constants.h"
#include "magic_bitboards.h"
#include "bb_helpers.h"
#include "board_declaration.h"
#include "uci.h"
#include "hashing.h"
#include "game_constants.h"
#include "make_move.h"
#include "eval_constants.h"
#include "move_ordering.h"
#include "perft.h"
#include "evaluate.h"
#include "search_position.h"

using namespace std;

// #include "Board.h"
// #include "Board.cpp"

void init_all()
{
    init_random_keys();
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
    clearTranspositionTable();
}

// rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
// 8/8/3p4/1Pp4r/1K3pk1/8/1R2P1P1/8 w - c6 0 1
int main()
{

    init_all();
    board.loadFenPosition(startpos);
    // cout << evaluate();
    uci_loop();

    return 0;
}
