#include "hashing.h"
#include <stdint.h>
#include "constants.h"
#include "game_constants.h"
#include "magic_bitboards.h"
#include "board_declaration.h"
#include "bitboard.h"
#include <iostream>

int hash_table_size = 64; //MB (64 MB default)
int hash_table_entries;// = hash_table_size*1024*1024/sizeof(tt); // 2796202 = 64MB

tt* transposition_table;

void init_hash_table() {
    hash_table_entries = hash_table_size*1024*1024/sizeof(tt);
    int bytes = hash_table_entries*sizeof(tt);
    transposition_table = (tt*)malloc(bytes);
    currentAge = 0;

    clearTranspositionTable();
}

// initialize / clear transposition table
void clearTranspositionTable()
{
    currentAge = 0;
    memset(transposition_table, 0, hash_table_entries * sizeof(tt));
    std::cout << "Hash table Initialized with size " << hash_table_size << " MB\n\n";
}

uint64_t side_key = 0, hash_key = 0;
uint64_t piece_keys[2][6][64];
// random en passant keys
uint64_t en_passant_keys[64];
uint64_t castle_keys[16];

// generate the castle key index for Zobrist hashing based on castling rights
int generate_castle_key_index()
{
    int castle_idx = 0;
    castle_idx += canWhiteCastleKing(board.boardSpecs) ? 8 : 0;
    castle_idx += canWhiteCastleQueen(board.boardSpecs) ? 4 : 0;
    castle_idx += canBlackCastleKing(board.boardSpecs) ? 2 : 0;
    castle_idx += canBlackCastleQueen(board.boardSpecs) ? 1 : 0;
    return castle_idx;
}

// generate Zobrist hash key of the current position
uint64_t generate_hash_key()
{
    uint64_t final_key = 0ULL;

    // loop over pieces
    for (int square = 0; square < 64; square++)
    {
        int pieceTypeOnWhite = board.getWhitePieceTypeOnSquare(square);
        int pieceTypeOnBlack = board.getBlackPieceTypeOnSquare(square);
        if (pieceTypeOnWhite != -1)
            final_key ^= piece_keys[1][pieceTypeOnWhite][square];
        if (pieceTypeOnBlack != -1)
            final_key ^= piece_keys[0][pieceTypeOnBlack][square];
    }

    if (getEnPassantSquare(board.boardSpecs) != -1)
        // hash en passant
        final_key ^= en_passant_keys[getEnPassantSquare(board.boardSpecs)];

    // hash castling rights
    int castle_idx = generate_castle_key_index();

    final_key ^= castle_keys[castle_idx];

    // hash the side only if black is to move
    if (board.colorToMove == 0)
        final_key ^= side_key;

    return final_key;
}

// init random hash keys for Zobrist hashing
void init_random_keys()
{

    // random state (seed)
    // unsigned int random_state = 1804289383; //already inintialized above

    // loop over pieces
    for (int piece = K; piece <= P; piece++)
    {
        for (int square = 0; square < 64; square++)
        {
            // init random piece keys
            piece_keys[0][piece][square] = random_uint64_t();
            piece_keys[1][piece][square] = random_uint64_t();
        }
    }
    for (int square = 0; square < 64; square++)
        en_passant_keys[square] = random_uint64_t();
    

    side_key = random_uint64_t();

    for (int castle = 0; castle < 16; castle++)
        castle_keys[castle] = random_uint64_t();
    
}

// read entry from the transposition table
tt* readHashEntry(int depth, int alpha, int beta, MOVE &best_move)
{
    // addressing the location of the entry we want to read
    tt *hash_entry = transposition_table + (hash_key % hash_table_entries);

    // make sure we got the exact position that we need
    // we start by comparing the current hash key with the one stored in the address
    if (hash_entry->hash_key == hash_key)
    {
        best_move = hash_entry->best_move;
        if (hash_entry->value < -MATE_SCORE)
                hash_entry->value += ply;
        if (hash_entry->value > MATE_SCORE)
            hash_entry->value -= ply;
        return hash_entry;
        
    }
    return nullptr;
}

// write hash entry in the transposition table
void writeHashEntry(int depth, int evaluation, MOVE best_move, int hash_flag)
{
    // address of the position in the transposition table we want to write in
    tt *hash_entry = transposition_table + (hash_key % hash_table_entries);

    bool replace = !hash_entry->hash_key || (hash_entry->age != currentAge || hash_entry->depth <= depth);

    if(!replace) return;

    // adjust the evaluation in case of mates
    // store the score independent from path from root to current node,
    if (evaluation < -MATE_SCORE)
        evaluation -= ply;
    if (evaluation > MATE_SCORE)
        evaluation += ply;

    hash_entry->depth = (uint8_t)depth;
    hash_entry->value = (int16_t)evaluation;
    hash_entry->flag = (uint8_t)hash_flag;
    hash_entry->hash_key = hash_key;
    hash_entry->best_move = best_move;
    hash_entry->age = (uint8_t)currentAge;
}
