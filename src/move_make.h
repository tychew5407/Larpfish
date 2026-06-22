/* File: move_make.h
 * ------------------
 * This file contains the implementation for move making and move unmaking
 * on board types.
 */

#ifndef MOVE_MAKE_H
#define MOVE_MAKE_H

#include "move.h"
#include "board.h"

/* DEFINITIONS */
#define ROOK_KING_CASTLE 5 // To-squares for rooks upon king/queen castling (for white).
#define ROOK_QUEEN_CASTLE 3

/* Additional info is required to undo moves, encapsulated by the
 * undo_move_t struct.
 */
typedef struct {
    piece_t captured_piece;
    int ep_square;
    unsigned char castling;
    unsigned int halfmove_clock;
} undo_move_t;

/* Function: make_move
 * --------------------
 * The `make_move` function takes a pointer to a board struct and a move and modifies
 * the board according to the move.
 */
void make_move(board *b, move_t move);

/* Function: unmake_move
 * ----------------------
 * The `unmake_move` function takes a pointer to a board struct and a move that is
 * assumed to had just been made, and modifies the board to undo the move.
 */
void unmake_move(board *b, move_t move);

#endif
