
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

void init_all() {
    init_random_keys();
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
    clearTranspositionTable();
}


int main() {
    init_all();
    board.loadFenPosition(startpos);
    uci_loop();

    return 0;
}
