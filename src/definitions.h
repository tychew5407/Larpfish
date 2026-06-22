/* FILE: definitions.h
 * --------------------
 * This file contains definitions of general chess terms
 * used between other .c/.h files in this project.
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/* BOARD CONSTANTS */
#define NUM_SQUARES 64
#define SIDE_LEN 8
#define SIDE_LEN_POWER 3 // SIDE_LEN log 2
#define NUM_PIECES 6
#define NUM_SIDES 2
#define NUM_CASTLES 4
#define NUM_PROMOS 4

// Bit masks for relevant ranks/files
#define RANK_1 0xFF
#define RANK_4 0xFF000000
#define RANK_5 0xFF00000000
#define RANK_8 0xFF00000000000000
#define FILE_A 0x0101010101010101
#define FILE_H 0x8080808080808080

/* SHIFT CONSTANTS */
#define VERT_SHIFT 8
#define VERT_SHIFT_POWER 3 // VERT_SHIFT log 2
#define HORIZ_SHIFT 1

/* SEARCH CONSTANTS */
#define MAX_MOVES 218
#define MAX_PLY 128

/* PIECE ENUMS */
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

#endif
