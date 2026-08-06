/* File: evaluation.h
 * -------------------
 * This file contains the implementation for the evaluation function,
 * used for search to measure how optimal a given position is.
 *
 * This project uses Michniewski's simplified evaluation function.
 */

#ifndef EVALUATION_H
#define EVALUATION_H

#include <inttypes.h>
#include "bitboard.h"
#include "board.h"

/* DEFINITIONS */
#define PAWN_WEIGHT 100
#define KNIGHT_WEIGHT 320
#define BISHOP_WEIGHT 330
#define ROOK_WEIGHT 500
#define QUEEN_WEIGHT 900

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
