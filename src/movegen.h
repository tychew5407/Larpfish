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
#include "attack_tables.h"

/* DEFINITIONS */
#define KING_CASTLE_PATH (uint64_t)0x60
#define QUEEN_CASTLE_PATH (uint64_t)0xe

/* Function: generate_moves
 * -------------------------
 * The `generate_moves` function takes a move_t array and a board pointer and populates
 * the array with all pseudo-legal moves in the board position.
 */
void generate_moves(move_t *move_arr, size_t *length, board *board);

/* Function: is_in_check
 * ----------------------
 * The `is_in_check` function takes a board pointer and side, and outputs whether
 * the side's king is in check.
 */
bool is_in_check(board *b, side s);

/* Function: print_move_list
 * -------------------------
 * The `print_move_list` function takes a move_t array and its length and prints all
 * the moves in the list in a user-friendly manner. Used for debugging purposes.
 */
void print_move_list(move_t *move_arr, size_t length);

#endif
