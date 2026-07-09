/* File: board_ascii.h
 * --------------------
 * This file specifies characters associated with different board
 * ASCII representations, used for printing or parsing FEN or moves.
 */

#ifndef BOARD_ASCII_H
#define BOARD_ASCII_H

#include "definitions.h"

static const char PIECE_ASCII[NUM_PIECES] = {'P', 'N', 'B', 'R', 'Q', 'K'};
static const char CASTLE_ASCII[NUM_CASTLES] = {'q', 'k', 'Q', 'K'};
static const char SQUARE_ASCII[NUM_SQUARES][3] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

/* The `get_square_from_ascii` function returns an integer conversion
 * of the ascii square representation, assumed to be valid.
 */
static inline int get_square_from_ascii(const char *square) {
    return (square[0] - 'a') + (square[1] - '1') * SIDE_LEN;
}

#endif
