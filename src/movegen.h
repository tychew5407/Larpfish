/* File: movegen.h
 * ----------------
 * This file contains the implementation of pseudo-legal move generation,
 * used to generate a list of pseudo-legal moves to then search and play.
 */

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "definitions.h"
#include "bitboard.h"
#include "move.h"
#include "board.h"

/* Function: generate_moves
 * -------------------------
 * The `generate_moves` function takes a move_t array and a board pointer and populates
 * the array with all pseudo-legal moves in the board position.
 */
void generate_moves(move_t *move_arr, size_t *length, board *board);

#endif
