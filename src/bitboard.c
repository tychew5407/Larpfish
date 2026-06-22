/* File: bitboard.c
 * -------------------
 * For further comments, see "bitboard.h".
 */

#include <assert.h>
#include <stdio.h>
#include "bitboard.h"

// Pre-computed lookup table for bit indices, used for bit_scan.
static const int INDEX64[64] = {
    0,  1, 48,  2, 57, 49, 28,  3,
   61, 58, 50, 42, 38, 29, 17,  4,
   62, 55, 59, 36, 53, 51, 43, 22,
   45, 39, 33, 30, 24, 18, 12,  5,
   63, 47, 56, 27, 60, 41, 37, 16,
   54, 35, 52, 21, 44, 32, 23, 11,
   46, 26, 40, 15, 34, 20, 31, 10,
   25, 14, 19,  9, 13,  8,  7,  6
};

// Debrujin sequence, used for bit_scan.
static const uint64_t DEBRUJIN64 = 0x03f79d71b4cb0a89;

void set_bit(bitboard *bb, int square) {
    *bb |= (1ULL << square);
}

void clear_bit(bitboard *bb, int square) {
    *bb &= ~(1ULL << square);
}

bool get_bit(bitboard bb, int square) {
    return (bb & (1ULL << square)) > 0;
}

int get_file(int square) {
    return square & (SIDE_LEN - 1);
}

int get_rank(int square) {
    return square >> SIDE_LEN_POWER;
}

int bit_scan(bitboard bb) {
    assert (bb != 0);
    return INDEX64[((bb & -bb) * DEBRUJIN64) >> 58];
}

void print_bitboard(bitboard bb) {
    for (int row = SIDE_LEN - 1; row >= 0; row --) {
        for (int col = 0; col < SIDE_LEN; col ++) {
            printf("%d", ((bb & (1ULL << (row * SIDE_LEN + col))) > 0));
        }
        printf("\n");
    }
}
