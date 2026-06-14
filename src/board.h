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

// For indexing in the bitboards
#define WHITE 0
#define BLACK 1

// A bitboard type is simply an unsigned 64-bit integer.
typedef uint64_t bitboard;

/* The board struct contains 12 bitboards total: each piece member of the struct
 * is an array of 2 bitboards, with the zero index representing white and the one
 * index representing black.
 */
typedef struct {
    bitboard pawns[2];
    bitboard knights[2];
    bitboard bishops[2];
    bitboard rooks[2];
    bitboard queens[2];
    bitboard kings[2];
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
 * The `get_bit` function takes in a pointer to a bitboard and an int square
 * and returns the value of the bit corresponding to the square (using LERF-mapping)
 * of the bitboard via a boolean value.
 */
bool get_bit(bitboard *bb, int square);

/* Function: print_board
 * ----------------------
 * The `print_board` function takes in a pointer to a board struct and prints out
 * an ASCII representation of the board state.
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
 * the board to the starting position for standard chess.
 */
void initialize_board(board *b);

#endif
