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
#define NUM_CASTLES 4
#define NO_EN_PASSANT -1

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

/* The bitboard struct is simply a 64-bit bit array. */
typedef uint64_t bitboard;

/* The board struct contains 12 piece bitboards total, with the first dimension being the
 * associated piece indexed via piece_t, and the second dimension being the associated
 * side indexed via side.
 *
 * `play_side` specifies whose turn it currently is.
 *
 * `en_passant_square` specifies the square that is capturable via en passant, or -1 if
 * there is no such square.
 *
 * `castling` is a 4-bit bitarray specifying whether can White can castle kingside, White
 * can castle queenside, Black can castle kingside, and Black can castle queenside, respectively.
 *
 * `halfmove_clock` specifies a decimal number of half moves with respect to the 50 move draw rule.
 *
 * `fullmove_counter` specifies the number of full turns in the game.
 */
typedef struct {
    bitboard piece_bbs[NUM_PIECES][NUM_SIDES];
    side play_side;
    int ep_square;
    unsigned char castling;
    unsigned int halfmove_clock;
    unsigned int fullmove_counter;
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
 *
 * If p and s are valid, they become populated with the bitboard's associated piece/side.
 */
bitboard *get_bitboard_from_square(board *b, int square, piece_t *p, side *s);

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

/* Function: initialize_board
 * ---------------------------
 * The initialize_board takes in a pointer to a board struct and initializes
 * the board to a cleared position.
 */
void initialize_board(board *b);

#endif
