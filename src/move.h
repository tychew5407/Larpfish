/* File: move.h
 * --------------
 * This file contains the implementation of moves, using from-to move encoding.
 */

#ifndef MOVE_H
#define MOVE_H

#include <stdint.h>
#include "definitions.h"

/* DEFINITIONS */

#define PROMO_FLAG (1 << 3)
#define CAPTURE_FLAG (1 << 2)
#define SPECIAL_FLAG 3
#define SQ_MASK_LEN 6
#define FLAG_MASK_LEN 4
#define FROM_MASK (63 << (SQ_MASK_LEN + FLAG_MASK_LEN))
#define TO_MASK (63 << FLAG_MASK_LEN)
#define FLAG_MASK 15

#define NO_MOVE 0 // sentinel flag to represent a non-move.

/* Encoded move have 3 fields:
 *   - The first 6 MSB encode the from square.
 *   - The next 6 bits encode the to square.
 *   - The remaining 4 LSB encode any special flags corresponding to the move.
 */
typedef uint16_t move_t;

typedef enum {
    QUIET                = 0,
    DOUBLE_PAWN_PUSH     = 1,
    KING_CASTLE          = 2,
    QUEEN_CASTLE         = 3,
    CAPTURE              = 4,
    EP_CAPTURE           = 5,
    KNIGHT_PROMO         = 8,
    BISHOP_PROMO         = 9,
    ROOK_PROMO           = 10,
    QUEEN_PROMO          = 11,
    KNIGHT_PROMO_CAPTURE = 12,
    BISHOP_PROMO_CAPTURE = 13,
    ROOK_PROMO_CAPTURE   = 14,
    QUEEN_PROMO_CAPTURE  = 15
} move_flag;

/* FUNCTION PROTOTYPES */

/* Function: get_from
 * -------------------
 * The `get_from` function takes a move_t and outputs its corresponding from
 * square.
 */
static inline int get_from(move_t move) {
    return (move & FROM_MASK) >> (SQ_MASK_LEN + FLAG_MASK_LEN);
}

/* Function: get_to
 * -----------------
 * The `get_to` function takes a move_t and outputs its corresponding to square.
 */
static inline int get_to(move_t move) {
    return (move & TO_MASK) >> FLAG_MASK_LEN;
}

/* Function: get_flag
 * -------------------
 * The `get_flag` function takes a move_t and outputs its corresponding move_flag enum.
 */
static inline move_flag get_flag(move_t move) {
    return (move & FLAG_MASK);
}

/* Function: set_from
 * -------------------
 * The `set_from` function takes a move_t pointer `move` and new square and sets the from
 * square of `move` to the new square.
 */
static inline void set_from(move_t *move, int square) {
    *move &= ~FROM_MASK;
    *move |= square << (SQ_MASK_LEN + FLAG_MASK_LEN);
}

/* Function: set_to
 * -----------------
 * The `set_to` function takes a move_t pointer `move` and new square and sets the to
 * square of `move` to the new square.
 */
static inline void set_to(move_t *move, int square) {
    *move &= ~TO_MASK;
    *move |= square << (FLAG_MASK_LEN);
}

/* Function: set_flag
 * -------------------
 * The `set_flag` function takes a move_t pointer `move` and move_flag and sets the move flag
 * of `move` to the corresponding move_flag.
 */
static inline void set_flag(move_t *move, move_flag flag) {
    *move &= ~FLAG_MASK;
    *move |= flag;
}

/* Function: encode_move
 * ----------------------
 * The `encode_move` function takes from/to squares and move_flag, outputs
 * the corresponding encoded move type.
 */
static inline move_t encode_move(int from_sq, int to_sq, move_flag flag) {
    move_t result = 0;

    set_from(&result, from_sq);
    set_to(&result, to_sq);
    set_flag(&result, flag);

    return result;
}

/* Function: is_reversible
 * ------------------------
 * The `is_reversible` function
 */

#endif
