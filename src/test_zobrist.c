/* File: test_zobrist.c
 * ---------------------
 * This file implements a collision-rate test harness to
 * test the quality of the Zobrist keys generated. With 64-bit
 * Zobrist keys, we can expect a collision after about 2^32 or
 * ~4 billion positions. For the sake of memory, we test in 32-bit
 * chunks, in which we can expect a collision after 2^16 or ~65
 * thousand positions.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <inttypes.h>
#include "definitions.h"
#include "random.h"
#include "board.h"
#include "zobrist.h"

/* DEFINITIONS */

// Number of board states to test in the set.
#define NUM_TEST_POSITIONS 1000000
#define SHIFT_AMT 1
#define SLICE_MASK 0xFFFFFFFFULL

/* FUNCTION PROTOTYPES */
static void generate_random_pieces(board *b);

/* GLOBAL VARIABLES */
static xorshift64_state RNG;

int main() {
    xorshift64_init(&RNG, (uint64_t)time(NULL));
    
    uint8_t *zobrist_bitarray = calloc((1ULL << 32) / 8, sizeof(uint8_t));

    assert(zobrist_bitarray);

    size_t node_index = 0;
    size_t collisions = 0;

    bool in_progress = true;
    
    while (in_progress) {
        board cur_board;
        initialize_board(&cur_board);
        generate_random_pieces(&cur_board);

        for (int i = 0; i < NUM_SIDES && in_progress; i++) {
            cur_board.play_side = i;

            for (int j = 0; j < 16 && in_progress; j++) {
                cur_board.castling = j;

                for (int k = NO_EN_PASSANT; k < 8 && in_progress; k++) {
                    cur_board.ep_square = k;

                    zobrist_board cur_zb = (generate_zobrist_board(&cur_board) >> SHIFT_AMT) & SLICE_MASK;
                    size_t bitarray_index = cur_zb / 8;
                    uint8_t zb_mask = 1 << (cur_zb % 8);
                    
                    if (zobrist_bitarray[bitarray_index] & zb_mask) {
                        collisions ++;
                    } else {
                        zobrist_bitarray[bitarray_index] |= zb_mask;
                    }

                    node_index ++;

                    if (node_index >= NUM_TEST_POSITIONS) in_progress = false;

                    if (node_index % 10000 == 0) {
                        printf("%lu positions calculated: %lu collisions.\n", node_index, collisions);
                    }
                }
            }
        }

    }

    printf("TEST COMPLETE:\n");
    printf("TOTAL POSITIONS: %lu.\n", node_index);
    printf("TOTAL COLLISIONS: %lu.\n", collisions);

    free(zobrist_bitarray);
    
    return 0;
}

/* Function: generate_random_pieces
 * ---------------------------------
 * The `generate_random_pieces` function takes a pointer to an
 * initialized board and populates its piece bitboards randomly.
 */
static void generate_random_pieces(board *b) {
    for (int i = 0; i < NUM_PIECES; i++) {
        for (int j = 0; j < NUM_SIDES; j++) {
            b->piece_bbs[i][j] = xorshift64(&RNG);
        }
    }
}
