#ifndef MAGIC_BITBOARDS_H
#define MAGIC_BITBOARDS_H

#include <stdint.h>
#include "bitboard.h"

/*=======================================================*\
|  The following implementation for the magic bitboards   |
|      is taken from Maksim Korzh and his BBC engine,     |
|        which I thank for the clear implementation       |
\*=======================================================*/

enum
{
    rook,
    bishop
};

// rook rellevant occupancy bits
extern int rook_rellevant_bits[64];

// bishop rellevant occupancy bits
extern int bishop_rellevant_bits[64];

// just a random number
extern unsigned int state;// = 1804289383;

// 32-bit number pseudo random generator
unsigned int generate_random_number();

// generate random uint64_t number
uint64_t random_uint64_t();

// get random few bits
// get random few bits
static inline uint64_t random_fewbits()
{
    return random_uint64_t() & random_uint64_t() & random_uint64_t();
}
// get index of LS1B in bitboard
// get index of LS1B in bitboard
static inline int get_ls1b_index(uint64_t bitboard)
{
    // make sure bitboard is not empty
    return (bitboard ? count_bits((bitboard & -bitboard) - 1) : -1);
}

// set occupancies
uint64_t set_occupancy(int index, int bits_in_mask, uint64_t attack_mask);

// mask bishop attacks
static inline uint64_t mask_bishop_attacks(int square);

// mask rook attacks
static inline uint64_t mask_rook_attacks(int square);
// bishop attacks
static inline uint64_t bishop_attacks_on_the_fly(int square, uint64_t block);

// rook attacks
static inline uint64_t rook_attacks_on_the_fly(int square, uint64_t block);
/**************************************\
        Generating magic numbers
\**************************************/

// find magic number
uint64_t find_magic(int square, int relevant_bits, int bishop);

/**************************************\
        Initializing attack table

\**************************************/

// masks
extern uint64_t bishop_masks[64];
extern uint64_t rook_masks[64];

// attacks
extern uint64_t bishop_attacks[64][512];
extern uint64_t rook_attacks[64][4096];

// rook magic numbers
extern const uint64_t rook_magics[64];
// bishop magic number
extern const uint64_t bishop_magics[64];

// init slider pieces attacks
void init_sliders_attacks(int is_bishop);

// lookup bishop attacks
uint64_t get_bishop_attacks(int square, uint64_t occupancy);
// lookup rook attacks
uint64_t get_rook_attacks(int square, uint64_t occupancy);

#endif