/* File: evaluation.c
 * -------------------
 * For more information, see "evaluation.h".
 */

#include <inttypes.h>
#include "evaluation.h"

/* List of constants for material counting. */
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


/* Piece-square tables */
const int PAWN_PST[NUM_SQUARES] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10,-20,-20, 10, 10,  5,
    5, -5,-10,  0,  0,-10, -5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5,  5, 10, 25, 25, 10,  5,  5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    0,  0,  0,  0,  0,  0,  0,  0
};

const int KNIGHT_PST[NUM_SQUARES] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int BISHOP_PST[NUM_SQUARES] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

const int ROOK_PST[NUM_SQUARES] = {
    0,  0,  0,  5,  5,  0,  0,  0,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
    5, 10, 10, 10, 10, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

const int QUEEN_PST[NUM_SQUARES] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -10,  5,  5,  5,  5,  5,  0,-10,
      0,  0,  5,  5,  5,  5,  0, -5,
     -5,  0,  5,  5,  5,  5,  0, -5,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

const int KING_MIDGAME_PST[NUM_SQUARES] = {
    20, 30, 10,  0,  0, 10, 30, 20,
    20, 20,  0,  0,  0,  0, 20, 20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30
};

const int KING_ENDGAME_PST[NUM_SQUARES] = {
    -50,-30,-30,-30,-30,-30,-30,-50,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -50,-40,-30,-20,-20,-30,-40,-50
};

/* GLOBAL VARIABLES */

// Piece counts, ignoring king since there is always one.
int white_count[NUM_PIECES - 1];
int black_count[NUM_PIECES - 1];
const int *piece_count[NUM_SIDES] = {white_count, black_count};
const int *piece_pst[NUM_PIECES - 1] = {PAWN_PST, KNIGHT_PST, BISHOP_PST, ROOK_PST, QUEEN_PST};

/* Function: flip_sq
 * ------------------
 * The `flip_sq` function returns the square on the other side
 * of `sq`.
 */
static inline int flip_sq(int sq) {
    return sq ^ 56;
}

/* Function: choose_king_pst
 * -------------------------
 * The `choose_king_pst` function returns the associated king
 * PST to use based on the game phase. Assumes material_count
 * to be updated.
 */
static inline const int *choose_king_pst() {
    for (int s = 0; s < NUM_SIDES; s++) {
        if (piece_count[s][QUEEN] > 0 ||
            piece_count[s][ROOK] > 0 ||
            piece_count[s][KNIGHT] + piece_count[s][BISHOP] > 1) {
            return KING_MIDGAME_PST;
        }
    }
    
    return KING_ENDGAME_PST;
}

int16_t evaluate(board *b) {
    int16_t result = 0;
    side s = b->play_side;

    for (int i = 0; i < NUM_PIECES - 1; i++) {
        white_count[i] = __builtin_popcountll(b->piece_bbs[i][WHITE]);
        black_count[i] = __builtin_popcountll(b->piece_bbs[i][BLACK]);
        result += get_piece_val(i) * (white_count[i] - black_count[i]);

        bitboard cur_white_bb = b->piece_bbs[i][WHITE];
        while (cur_white_bb) {
            int sq = bit_scan_forward(cur_white_bb);
            result += piece_pst[i][sq];
            cur_white_bb &= cur_white_bb - 1;
        }

        bitboard cur_black_bb = b->piece_bbs[i][BLACK];
        while (cur_black_bb) {
            int sq = flip_sq(bit_scan_forward(cur_black_bb));
            result -= piece_pst[i][sq];
            cur_black_bb &= cur_black_bb - 1;
        }
    }

    // Consider King PST
    int white_king_sq = bit_scan_forward(b->piece_bbs[KING][WHITE]);
    int black_king_sq = flip_sq(bit_scan_forward(b->piece_bbs[KING][BLACK]));
    const int *cur_king_pst = choose_king_pst();

    result += (cur_king_pst[white_king_sq] - cur_king_pst[black_king_sq]);
    result *= SIDE_WEIGHT[s];
    
    return result;
}
