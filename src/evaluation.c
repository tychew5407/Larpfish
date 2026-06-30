/* File: evaluation.c
 * -------------------
 * For more information, see "evaluation.h".
 */

#include "evaluation.h"

// List of cnstants for material counting
const int MATERIAL[5] = {
    PAWN_WEIGHT,
    KNIGHT_WEIGHT,
    BISHOP_WEIGHT,
    ROOK_WEIGHT,
    QUEEN_WEIGHT
};

const int SIDE_WEIGHT[2] = {
    1, // WHITE
    -1 // BLACK
};

int evaluate(board *b) {
    int result = 0;
    side s = b->play_side;

    for (int i = 0; i < NUM_PIECES - 1; i++) {
        int w_count = __builtin_popcountll(b->piece_bbs[i][WHITE]);
        int b_count = __builtin_popcountll(b->piece_bbs[i][BLACK]);
        result += MATERIAL[i] * (w_count - b_count) * SIDE_WEIGHT[s];
    }

    return result;
}
