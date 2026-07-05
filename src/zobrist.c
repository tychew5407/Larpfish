/* File: zobrist.c
 * ----------------
 * For more information, see "zobrist.h".
 */

#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include "bitboard.h"
#include "zobrist.h"

void generate_zobrist_keys() {
    uint64_t zobrist_nums[ZOBRIST_ARR_SIZE];
    xorshift64_state rng;
    xorshift64_init(&rng, (uint64_t)time(NULL));

    printf("{\n\t");
    for (int i = 1; i <= ZOBRIST_ARR_SIZE; i++) {
        zobrist_nums[i-1] = xorshift64(&rng);
        printf("0x%016" PRIx64, zobrist_nums[i-1]);
        if (i < ZOBRIST_ARR_SIZE) printf(", ");
        if (i % 4 == 0) printf("\n\t");
    }
    printf("\n}\n");
}

zobrist_board generate_zobrist_board(board *b) {
    zobrist_board result = 0;

    for (int i = 0; i < NUM_PIECES; i++) {
        for (int j = 0; j < NUM_SIDES; j++) {
            bitboard cur_bb = b->piece_bbs[i][j];
            while (cur_bb) {
                int sq = bit_scan_forward(cur_bb);
                toggle_zobrist_piece(&result, i, j, sq);
                cur_bb &= cur_bb - 1;
            }
        }
    }

    if (b->play_side == BLACK) {
        toggle_zobrist_turn(&result);
    }

    toggle_zobrist_castling(&result, b->castling);

    if (b->ep_square != NO_EN_PASSANT) {
        toggle_zobrist_ep(&result, get_file(b->ep_square));
    }
    
    return result;
}
