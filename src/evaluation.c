/* File: evaluation.c
 * -------------------
 * For more information, see "evaluation.h".
 */

#include <inttypes.h>
#include "evaluation.h"

// List of constants for material counting
const int MATERIAL[NUM_PIECES - 1] = {
    PAWN_WEIGHT,
    KNIGHT_WEIGHT,
    BISHOP_WEIGHT,
    ROOK_WEIGHT,
    QUEEN_WEIGHT
};

const int SIDE_WEIGHT[NUM_SIDES] = {
    1, // WHITE
    -1 // BLACK
};

int16_t evaluate(board *b) {
    int16_t result = 0;
    side s = b->play_side;

    for (int i = 0; i < NUM_PIECES - 1; i++) {
        int w_count = __builtin_popcountll(b->piece_bbs[i][WHITE]);
        int b_count = __builtin_popcountll(b->piece_bbs[i][BLACK]);
        result += get_piece_val(i) * (w_count - b_count) * SIDE_WEIGHT[s];
    }

    return result;
}
