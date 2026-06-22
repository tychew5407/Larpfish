/* File: board.c
 * --------------
 * For further comments, see "board.h".
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"

/* Used for printing or parsing FEN. */
const char PIECE_ASCII[NUM_PIECES] = {'P', 'N', 'B', 'R', 'Q', 'K'};
const char CASTLE_ASCII[NUM_CASTLES] = {'q', 'k', 'Q', 'K'};

void initialize_board(board *b) {
    for (int i = 0; i < NUM_SIDES; i++) {
        for (int j = 0; j < NUM_PIECES; j++) {
            b->piece_bbs[j][i] = 0;
        }

        b->occupied_bbs[i] = 0;
    }

    b->play_side = WHITE;
    b->ep_square = NO_EN_PASSANT;
    b->castling = 0;
    b->halfmove_clock = 0;
    b->fullmove_counter = 0;
}

void print_board(board *b) {
    for (int row = SIDE_LEN - 1; row >= 0; row--) {
        for (int col = 0; col < SIDE_LEN; col++) {
            int square = col + (row * SIDE_LEN);
            piece_t curr_piece;
            side curr_side;
            bitboard *curr = get_bitboard_from_square(b, square, &curr_piece, &curr_side);

            if (curr) {
                int offset = ('a' - 'A') * curr_side;
                printf("%c", PIECE_ASCII[curr_piece] + offset);
            } else {
                printf("-");
            }
        }
        printf("\n");
    }
}

bitboard *get_bitboard_from_square(board *b, int square, piece_t *p, side *s) {
    for (int i = 0; i < NUM_PIECES; i++) {
        for (int j = 0; j < NUM_SIDES; j++) {
            if (get_bit(b->piece_bbs[i][j], square)) {
                if (p) {
                    *p = i;
                }

                if (s) {
                    *s = j;
                }

                return &(b->piece_bbs[i][j]);
            }
        }
    }

    return NULL;
}

bitboard *get_bitboard_from_ascii(board *b, char piece_c, side *s) {
    for (int i = 0; i < NUM_PIECES; i++) {
        if (PIECE_ASCII[i] == piece_c ||
            PIECE_ASCII[i] + ('a' - 'A') == piece_c) {
            if (s) {
                *s = (PIECE_ASCII[i] + ('a' - 'A') == piece_c);
            }
            
            return &(b->piece_bbs[i][*s]);
        }
    }

    return NULL;
}
