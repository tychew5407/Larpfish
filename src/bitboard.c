/* File: bitboard.c
 * -------------------
 * For further comments, see "bitboard.h".
 */

#include <stdio.h>
#include "bitboard.h"

/* void set_bit(bitboard *bb, int square) { */
/*     *bb |= (1ULL << square); */
/* } */

/* void clear_bit(bitboard *bb, int square) { */
/*     *bb &= ~(1ULL << square); */
/* } */

/* bool get_bit(bitboard bb, int square) { */
/*     return (bb & (1ULL << square)) > 0; */
/* } */

/* int get_file(int square) { */
/*     return square & (SIDE_LEN - 1); */
/* } */

/* int get_rank(int square) { */
/*     return square >> SIDE_LEN_POWER; */
/* } */

void print_bitboard(bitboard bb) {
    for (int row = SIDE_LEN - 1; row >= 0; row --) {
        for (int col = 0; col < SIDE_LEN; col ++) {
            printf("%d", ((bb & (1ULL << (row * SIDE_LEN + col))) > 0));
        }
        printf("\n");
    }
}
