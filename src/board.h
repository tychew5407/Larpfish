/* File: board.h
 * --------------
 * This file contains the implementation of the board struct, which will
 * contain information about the state of the board and the game.
 *
 * This project uses bitboards with little endian rank-file mapping (LERF-mapping)
 * to represent the board.
 */

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

/* DEFINITIONS */

#define NUM_SQUARES 64
#define SIDE_LEN 8
#define NUM_PIECES 6
#define NUM_SIDES 2

// For indexing in the bitboards
typedef enum {
    WHITE,
    BLACK
} side;

typedef enum {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
} piece_t;

/* The bitboard struct contains the associated piece type, side, and its value. */
typedef struct {
    piece_t piece;
    side side;
    uint64_t value;
} bitboard;

/* The board struct contains 12 bitboards total, with the first dimension being the
 * associated piece indexed via piece_t, and the second dimension being the associated
 * side indexed via side.
 */
typedef struct {
    bitboard bitboards[NUM_PIECES][NUM_SIDES];
} board;

/* FUNCTION PROTOTYPES */

/* Function: set_bit
 * ------------------
 * The `set_bit` function takes in a pointer to a bitboard and an int square
 * and sets the bit corresponding to the square (using LERF-mapping) of the bitboard
 * to 1. Assumes `bb` points to a valid bitboard and that `square` is between 0 and 63.
 */
void set_bit(bitboard *bb, int square);

/* Function: clear_bit
 * --------------------
 * The `clear_bit` function takes in a pointer to a bitboard and an int square
 * and sets the bit corresponding to the square (using LERF-mapping) of the bitboard
 * to 0.
 */
void clear_bit(bitboard *bb, int square);

/* Function: get_bit
 * ------------------
 * The `get_bit` function takes in a bitboard and an int square and returns the
 * value of the bit corresponding to the square (using LERF-mapping) of the bitboard
 * via a boolean value.
 */
bool get_bit(bitboard bb, int square);

/* Function: get_bitboard_from_square
 * -----------------------------------
 * The `get_bitboard_from_square` function takes in a valid `square` value and outputs
 * the corresponding bitboard from `board` that holds a piece on that square. Returns NULL
 * if no such bitboard is found.
 */
bitboard *get_bitboard_from_square(board *b, int square);

/* Function: get_bitboard_from_ascii
 * ----------------------------------
 * The `get_bitboard_from_ascii` function takes in an ASCII representation of a piece
 * `piece_c` and outputs the corresponding pointer to the bitboard from `board`, or NULL
 * if piece_c is invalid.
 */
bitboard *get_bitboard_from_ascii(board *b, char piece_c);

/* Function: print_board
 * ----------------------
 * The `print_board` function takes in a pointer to a board struct and prints out
 * an ASCII representation of the board state. White pieces will print in uppercase
 * letters while black pieces will print in lowercase.
 */
void print_board(board *b);

/* Function: parse_fen
 * --------------------
 * The `parse_fen` function takes in a pointer to a board struct and an encoded
 * FEN string and sets the board struct to align with the FEN values.
 */
void parse_fen(board *b, char *fen);

/* Function: initialize_board
 * ---------------------------
 * The initialize_board takes in a pointer to a board struct and initializes
 * the board to a cleared position.
 */
void initialize_board(board *b);

#endif
