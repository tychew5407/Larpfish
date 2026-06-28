/* FILE: definitions.h
 * --------------------
 * This file contains definitions of general chess terms
 * used between other .c/.h files in this project.
 */

#include <stdint.h>

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

// Bit masks for ranks/files
#define RANK_1 (uint64_t)0xFF
#define RANK_2 (RANK_1 << SIDE_LEN)
#define RANK_3 (RANK_2 << SIDE_LEN)
#define RANK_4 (RANK_3 << SIDE_LEN)
#define RANK_5 (RANK_4 << SIDE_LEN)
#define RANK_6 (RANK_5 << SIDE_LEN)
#define RANK_7 (RANK_6 << SIDE_LEN)
#define RANK_8 (RANK_7 << SIDE_LEN)

#define FILE_A 0x0101010101010101
#define FILE_B (FILE_A << 1)
#define FILE_C (FILE_B << 1)
#define FILE_D (FILE_C << 1)
#define FILE_E (FILE_D << 1)
#define FILE_F (FILE_E << 1)
#define FILE_G (FILE_F << 1)
#define FILE_H (FILE_G << 1)

/* SHIFT CONSTANTS */
#define VERT_SHIFT SIDE_LEN
#define VERT_SHIFT_POWER SIDE_LEN_POWER // VERT_SHIFT log 2
#define HORIZ_SHIFT 1

/* SEARCH CONSTANTS */
#define MAX_MOVES 218
#define MAX_PLY 128

/* PIECE ENUMS */
typedef enum {
    WHITE,
    BLACK,
} side;

typedef enum {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NO_PIECE = -1
} piece_t;

#endif
