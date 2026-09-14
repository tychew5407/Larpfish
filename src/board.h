/* File: board.h
 * --------------
 * This file contains the implementation of the board struct, which will
 * contain information about the state of the board and the game.
 *
 * This project uses a redundant hybrid representation with bitboards using
 * little endian rank-file mapping (LERF-mapping) and piece/side mailboxes to
 * represent the board.
 */

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "definitions.h"
#include "bitboard.h"
#include "move.h"

/* DEFINITIONS */

#define NO_EN_PASSANT -1 // Sentinel flag for ep_square
#define CASTLE_ARR_START (1 << 3) // Mask to access start of castling bitarray

/* The board struct contains 12 piece bitboards total, with the first dimension being the
 * associated piece indexed via piece_t, and the second dimension being the associated
 * side indexed via side. The board struct additionally has two union bitboards that
 * contain the occupied pieces of each side.
 *
 * Other members:
 * - `play_side` specifies whose turn it currently is.
 * - `en_passant_square` specifies the square that is capturable via en passant, or -1 if
 *   there is no such square.
 * - `castling` is a 4-bit bitarray specifying whether can White can castle kingside,
 *   White can castle queenside, Black can castle kingside, and Black can castle queenside,
 *   respectively.
 * - `halfmove_clock` specifies a decimal number of half moves with respect to the 50 move
 *   draw rule.
 * - `fullmove_counter` specifies the number of full turns in the game.
 */
typedef struct {
    bitboard piece_bbs[NUM_PIECES][NUM_SIDES];
    bitboard occupied_bbs[NUM_SIDES];
    
    piece_t piece_mailbox[NUM_SQUARES];
    side side_mailbox[NUM_SQUARES];
    
    side play_side;
    int ep_square;
    unsigned char castling; // WK WQ BK BQ
    unsigned int halfmove_clock;
    unsigned int fullmove_counter;
} board;

/* FUNCTION PROTOTYPES */

/* Function: initialize_board
 * ---------------------------
 * The `initialize_board` function takes in a pointer to a board struct and initializes
 * the board to a cleared position.
 */
void initialize_board(board *b);

/* Function: print_board
 * ----------------------
 * The `print_board` function takes in a pointer to a board struct and prints out
 * an ASCII representation of the board state. White pieces will print in uppercase
 * letters while black pieces will print in lowercase.
 */
void print_board(board *b);

/* Function: piece_on
 * -------------------
 * The `piece_on` function takes a pointer to a board struct and a square and outputs
 * the piece on that square, or NO_PIECE if no piece was found.
 */
static inline piece_t piece_on(const board *b, const int square) {
    return b->piece_mailbox[square];
}

/* Function: side_on
 * ------------------
 * The `side_on` function takes a pointer to a board struct and a square and outputs
 * the side of the piece on that square. Assumes that there is a piece on the given
 * square.
 */
static inline side side_on(const board *b, const int square) {
    return b->side_mailbox[square];
}

/* Function: get_bitboard_from_square
 * -----------------------------------
 * The `get_bitboard_from_square` function takes in a valid `square` value and outputs
 * the corresponding bitboard from `board` that holds a piece on that square. Returns NULL
 * if no such bitboard is found.
 *
 * If p and s are valid, they become populated with the bitboard's associated piece/side.
 */
static inline bitboard *get_bitboard_from_square(board *b, int square) {
    piece_t bb_piece = b->piece_mailbox[square];
    if (bb_piece == NO_PIECE) return NULL;
    return &(b->piece_bbs[bb_piece][b->side_mailbox[square]]);
}

/* Function: get_bitboard_from_ascii
 * ----------------------------------
 * The `get_bitboard_from_ascii` function takes in an ASCII representation of a piece
 * `piece_c` and outputs the corresponding pointer to the bitboard from `board`, or NULL
 * if piece_c is invalid.
 *
 * If s is valid, it is populated with the bitboard's associated side.
 */
bitboard *get_bitboard_from_ascii(board *b, char piece_c, piece_t *p, side *s);

/* Function: board_cmp
 * --------------------
 * The `board_cmp` function takes two boards and returns whether they are
 * equal to each other or not. Used for debugging purposes.
 */
bool board_cmp(board a, board b);

#endif
