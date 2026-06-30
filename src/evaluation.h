/* File: evaluation.h
 * -------------------
 * This file contains the implementation for the evaluation function,
 * used for search to measure how optimal a given position is.
 *
 * This project uses pure material counting for evaluation to start.
 */

#ifndef EVALUATION_H
#define EVALUATION_H

#include "bitboard.h"
#include "board.h"

/* DEFINITIONS */
#define PAWN_WEIGHT 1
#define KNIGHT_WEIGHT 3
#define BISHOP_WEIGHT 3
#define ROOK_WEIGHT 5
#define QUEEN_WEIGHT 9

int evaluate(board *b);

#endif
