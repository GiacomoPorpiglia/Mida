#include "magic_bitboards.h"
#include "board_declaration.h"
#include "hashing.h"
#include "uci.h"
#include "evaluate.h"
#include "constants.h"
#include <iostream>

using namespace std;

void init_all() {
    init_random_keys();
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
    clearTranspositionTable();
}

//__declspec(dllexport)
int main() {
    init_all();
    board.loadFenPosition(startpos);
    uci_loop();

    return 0;
}
