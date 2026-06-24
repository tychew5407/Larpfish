/* File: bitboard.h
 * -----------------
 * This file contains the definition and helper functions related
 * to bitboards.
 */

#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>
#include <stdbool.h>
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
void set_bit(bitboard *bb, int square);

/* Function: clear_bit
 * --------------------
 * The `clear_bit` function takes in a pointer to a bitboard and an int square
 * and sets the bit corresponding to the square (using LERF-mapping) of the bitboard
 * to 0.
 */
void clear_bit(bitboard *bb, int square);

/* Function: get_bit
 * ------------------
 * The `get_bit` function takes in a bitboard and an int square and returns the
 * value of the bit corresponding to the square (using LERF-mapping) of the bitboard
 * via a boolean value.
 */
bool get_bit(bitboard bb, int square);

/* Function: get_file
 * -------------------
 * The `get_file` function takes an int square value and outputs its corresponding
 * file (zero-based, i.e. 0-7).
 */
int get_file(int square);

/* Function: get_rank
 * --------------------
 * The `get_rank` function takes an int square value and outputs its corresponding
 * rank (zero-based, i.e. 0-7).
 */
int get_rank(int square);

/* Function: bit_scan
 * -------------------
 * The `bit_scan` function takes a bitboard value and outputs the index of its LS1B.
 * Implemented using De Bruijn multiplication. Assumes that bb != 0.
 */
int bit_scan(bitboard bb);

/* Function: bit_scan_reverse
 * -------------------
 * The `bit_scan_reverse` function takes a bitboard value and outputs the index of its MS1B.
 * Implemented using De Bruijn multiplication. Assumes that bb != 0.
 */
int bit_scan_reverse(bitboard bb);

/* Function: print_bitboard
 * -------------------------
 * The `print_bitboard` function takes a bitboard value and prints out the bits
 * in a user-friendly LERF-mapped board fashion. Used for debugging purposes.
 */
void print_bitboard(bitboard bb);

#endif
