#include "magic_bitboards.h"
#include "board_declaration.h"
#include "hashing.h"
#include "uci.h"
#include "search.h"
#include "nnue_eval.h"
#include "constants.h"
#include "nnue.h"
// #include <iostream>

// using namespace std;

void init_all() {

    nnue_export_net();
    init_hash_table();
    init_random_keys();
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
    init_search();
}

//r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 

///8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 

//r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1

//8/kpp5/p4Q2/P1P5/KP5r/4R1p1/5q2/8 w - - 0 1


int main() {
    init_all();
    board.loadFenPosition(startpos);
    char *filename = "mida-nn.nnue";
    init_nnue(filename);

    //cout << "Initial position NNUE evaluation: " << evaluate_fen_nnue(startpos) << evaluate();
    uci_loop();

    return 0;
}
