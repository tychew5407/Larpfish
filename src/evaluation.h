/* File: evaluation.h
 * -------------------
 * This file contains the implementation for the evaluation function,
 * used for search to measure how optimal a given position is.
 *
 * This project uses pure material counting for evaluation to start.
 */

#ifndef EVALUATION_H
#define EVALUATION_H

#include <inttypes.h>
#include "bitboard.h"
#include "board.h"

/* DEFINITIONS */
#define PAWN_WEIGHT 1
#define KNIGHT_WEIGHT 3
#define BISHOP_WEIGHT 3
#define ROOK_WEIGHT 5
#define QUEEN_WEIGHT 9

extern const int MATERIAL[NUM_PIECES - 1];

/* Function: get_piece_val
 * -----------------------
 * The `get_piece_val` function takes a piece_t and returns its
 * associated piece value weight.
 */
static inline int get_piece_val(piece_t p) {
    return MATERIAL[p];
}

/* Function: evaluate
 * ------------------
 * The `evaluate` function takes in a pointer to a board and
 * returns a score representing the evaluation of the given
 * position.
 */
int16_t evaluate(board *b);

#endif
