/* File: zobrist.h
 * ----------------
 * This file contains the implementation for getting Zobrist hashes
 * from board positions and updating them incrementally from moves.
 *
 * Zobrist hashes are used for draw by repetition and transposition
 * tables.
 */

#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <stdint.h>
#include "definitions.h"
#include "board.h"
#include "random.h"
#include "zobrist_keys.h"

/* DEFINITIONS */

#define TURN_INDEX (12 * 64)
#define CASTLE_INDEX (TURN_INDEX + 1)
#define EP_INDEX (CASTLE_INDEX + 16)

typedef uint64_t zobrist_board;

/* The `generate_zobrist_keys` function prints out a candidate
 * zobrist_keys array with 793 random numbers:
 *   - One number for each piece on each square.
 *   - One number to indicate whether it is black's turn to move.
 *   - 16 numbers to indicate each combination of castling rights.
 *   - Eight numbers to indicate the file of the en passant square.
 *
 * Each relevant number representing the board position is XORed together
 * to generate a hashed 64-bit number.
 */
void generate_zobrist_keys();

/* The `generate_zobrist_board` function takes in a board position and
 * outputs its corresponding zobrist hash. Should be used only on new
 * positions. For updating positions, use the toggling functions.
 */
zobrist_board generate_zobrist_board(board *b);

/* The `toggle_zobrist_turn` function takes a current zobrist_board and
 * XORs it with the number corresponding to whether it is black's turn.
 */
static inline void toggle_zobrist_turn(zobrist_board *zb) {
    *zb ^= zobrist_keys[TURN_INDEX];
};

/* The `toggle_zobrist_castling` function takes a current zobrist_board and
 * XORs it with the number corresponding to the bit-array `castling`
 * with the input zobrist_board.
 */
static inline void toggle_zobrist_castling(zobrist_board *zb, unsigned char castling) {
    *zb ^= zobrist_keys[CASTLE_INDEX + castling];
}

/* The `toggle_zobrist_ep` function takes a current zobrist_board and
 * XORs it with the number corresponding to the file of the en passant
 * square `ep_file`.
 */
static inline void toggle_zobrist_ep(zobrist_board *zb, int ep_file) {
    *zb ^= zobrist_keys[EP_INDEX + ep_file];
}

/* The `toggle_zobrist_piece` function takes a current zobrist_board and
 * XORs it with the number corresponding to the piece, side, and square.
 */
static inline void toggle_zobrist_piece(zobrist_board *zb, piece_t piece, side side, int square) {
    size_t piece_index = (piece * NUM_SIDES + side) * NUM_SQUARES + square;
    *zb ^= zobrist_keys[piece_index];
}

#endif
