#ifndef HASHING_H
#define HASHING_H

#include <stdint.h>
#include "constants.h"
#include "game_constants.h"

//--------------ZOBRIST HASHING---------------------

// random piece keys
extern uint64_t piece_keys[2][6][64];
// random en passant keys
extern uint64_t en_passant_keys[64];
// random castle keys
extern uint64_t castle_keys[16];
// random side key
extern uint64_t side_key;

extern uint64_t hash_key;

// Transposition table elements structure
typedef struct
{
    uint64_t hash_key; // hash key of the position
    int depth;         // current depth of the search
    int flag;          // flag the type of node (fail-high, fail-low, PV) . it will have the values of the HASH_FLAG constants
    int value;         // evaluation of the position
    MOVE best_move;    // best move
    
} tt;

// transposition table
extern tt transposition_table[HASH_TABLE_ENTRIES];

// initialize / clear transposition table
void clearTranspositionTable();

// generate the castle key index for Zobrist hashing based on castling rights
int generate_castle_key_index();

// generate Zobrist hash key of the current position
uint64_t generate_hash_key();
// init random hash keys for Zobrist hashing
void init_random_keys();

// read entry from the transposition table
tt* readHashEntry(int depth, int alpha, int beta, MOVE &best_move);
// write hash entry in the transposition table
void writeHashEntry(int depth, int evaluation, MOVE best_move, int hash_flag);

#endif