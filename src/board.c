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
    memset(b->piece_bbs, 0, sizeof(b->piece_bbs));
    memset(b->occupied_bbs, 0, sizeof(b->occupied_bbs));
    memset(b->piece_mailbox, NO_PIECE, sizeof(b->piece_mailbox));
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
    piece_t bb_piece = b->piece_mailbox[square];

    if (bb_piece == NO_PIECE) {
        return NULL;
    }
    
    side bb_side = b->side_mailbox[square];

    if (p) {
        *p = bb_piece;
    }

    if (s) {
        *s = bb_side;
    }

    return &(b->piece_bbs[bb_piece][bb_side]);
}

bitboard *get_bitboard_from_ascii(board *b, char piece_c, piece_t *p, side *s) {
    for (int i = 0; i < NUM_PIECES; i++) {
        if (PIECE_ASCII[i] == piece_c ||
            PIECE_ASCII[i] + ('a' - 'A') == piece_c) {
            if (p) {
                *p = i;
            }

            side bb_side = (PIECE_ASCII[i] + ('a' - 'A') == piece_c);
            
            if (s) {
                *s = bb_side;
            }
            
            return &(b->piece_bbs[i][bb_side]);
        }
    }

    return NULL;
}

bool board_cmp(board a, board b) {
    for (int i = 0; i < NUM_SIDES; i++) {
        for (int j = 0; j < NUM_PIECES; j++) {
            if (a.piece_bbs[j][i] != b.piece_bbs[j][i]) {
                return false;
            }
        }

        if (a.occupied_bbs[i] != b.occupied_bbs[i]) {
            return false;
        }
    }

    if (a.play_side != b.play_side ||
        a.ep_square != b.ep_square ||
        a.castling != b.castling ||
        a.halfmove_clock != b.halfmove_clock ||
        a.fullmove_counter != b.fullmove_counter) {
        return false;
    }

    return true;
}
