/* File: bitboard.h
 * -----------------
 * This file contains the definition and helper functions related
 * to bitboards.
 */

#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "definitions.h"

/* DEFINITIONS */

// The bitboard struct is simply a 64-bit bit array.
typedef uint64_t bitboard;

/* FUNCTION PROTOTYPES */

/* Function: set_bit
 * ------------------
 * The `set_bit` function takes in a pointer to a bitboard and an int square
 * and sets the bit corresponding to the square (using LERF-mapping) of the bitboard
 * to 1. Assumes `bb` points to a valid bitboard and that `square` is between 0 and 63.
 */
static inline void set_bit(bitboard *bb, int square) {
    *bb |= (1ULL << square);
}

/* Function: clear_bit
 * --------------------
 * The `clear_bit` function takes in a pointer to a bitboard and an int square
 * and sets the bit corresponding to the square (using LERF-mapping) of the bitboard
 * to 0.
 */
static inline void clear_bit(bitboard *bb, int square) {
    *bb &= ~(1ULL << square);
}

/* Function: get_bit
 * ------------------
 * The `get_bit` function takes in a bitboard and an int square and returns the
 * value of the bit corresponding to the square (using LERF-mapping) of the bitboard
 * via a boolean value.
 */
static inline bool get_bit(bitboard bb, int square) {
    return (bb & (1ULL << square)) > 0;
}

/* Function: get_file
 * -------------------
 * The `get_file` function takes an int square value and outputs its corresponding
 * file (zero-based, i.e. 0-7).
 */
static inline int get_file(int square) {
    return square & (SIDE_LEN - 1);
}

/* Function: get_rank
 * --------------------
 * The `get_rank` function takes an int square value and outputs its corresponding
 * rank (zero-based, i.e. 0-7).
 */
static inline int get_rank(int square) {
    return square / SIDE_LEN;
}

/* Function: bit_scan_foward
 * --------------------------
 * The `bit_scan_foward` function takes a bitboard and outputs the index of its LS1B.
 */
static inline int bit_scan_forward(bitboard bb) {
    assert(bb != 0);
    return __builtin_ctzll(bb);
}

/* Function: bit_scan_reverse
 * ---------------------------
 * The `bit_scan_reverse` function takes a bitboard and outputs the index of its MS1B.
 */
static inline int bit_scan_reverse(bitboard bb) {
    assert(bb != 0);
    return 63 - __builtin_clzll(bb);
}

/* Function: print_bitboard
 * -------------------------
 * The `print_bitboard` function takes a bitboard value and prints out the bits
 * in a user-friendly LERF-mapped board fashion. Used for debugging purposes.
 */
void print_bitboard(bitboard bb);

#endif
