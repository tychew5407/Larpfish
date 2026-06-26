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

/* DEFINITIONS */
#define KNIGHT_DIRS 8
#define KING_DIRS 8
#define KING_CASTLE_PATH (uint64_t)0x60
#define QUEEN_CASTLE_PATH (uint64_t)0xe
#define RAY_DIRS 8
#define NUM_SLIDERS 3

typedef enum {
    NORTH,
    NORTHEAST,
    EAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    WEST,
    NORTHWEST
} ray_dir;

typedef struct {
    ray_dir dir;
    size_t shift_amount;
    bool negative;
    bitboard wrap_check;
} ray;

/* Function: init_attack_tables
 * -----------------------------
 * The `init_attack_tables` function initializes the private lookup attack tables for
 * move generation. Should be called before any move generation.
 */
void init_attack_tables();

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
