/* File: board.c
 * --------------
 * For further comments, see "board.h".
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "board.h"

/* Used for printing or parsing FEN. */
const char PIECE_ASCII[NUM_PIECES] = {'P', 'N', 'B', 'R', 'Q', 'K'};

void set_bit(bitboard *bb, int square) {
    bb->value |= (1ULL << square);
}

void clear_bit(bitboard *bb, int square) {
    bb->value &= ~(1ULL << square);
}

bool get_bit(bitboard bb, int square) {
    return (bb.value & (1ULL << square)) > 0;
}

bitboard *get_bitboard_from_square(board *b, int square) {
    for (int i = 0; i < NUM_PIECES; i++) {
        for (int j = 0; j < NUM_SIDES; j++) {
            if (get_bit(b->bitboards[i][j], square)) {
                return &(b->bitboards[i][j]);
            }
        }
    }

    return NULL;
}

bitboard *get_bitboard_from_ascii(board *b, char piece_c) {
    for (int i = 0; i < NUM_PIECES; i++) {
        if (PIECE_ASCII[i] == piece_c ||
            PIECE_ASCII[i] + ('a' - 'A') == piece_c) {
            return &(b->bitboards[i][PIECE_ASCII[i] + ('a' - 'A') == piece_c]);
        }
    }

    return NULL;
}

void print_board(board *b) {
    for (int row = SIDE_LEN - 1; row >= 0; row--) {
        for (int col = 0; col < SIDE_LEN; col++) {
            int square = col + (row * SIDE_LEN);
            bitboard *curr = get_bitboard_from_square(b, square);

            if (curr) {
                int offset = ('a' - 'A') * curr->side;
                printf("%c", PIECE_ASCII[curr->piece] + offset);
            } else {
                printf("-");
            }
        }
        printf("\n");
    }
}

void parse_fen(board *b, char *fen) {
    char *space = strchr(fen, ' ');
    char *cur = fen;
    int square = SIDE_LEN * (SIDE_LEN - 1);

    while (cur != space) {
        if (*cur == '/') {
            square -= 2 * SIDE_LEN;
            cur ++;
            continue;
        }
        
        bitboard *cur_bb = get_bitboard_from_ascii(b, *cur);

        if (cur_bb) {
            set_bit(cur_bb, square);
            square ++;
        } else {
            square += *cur - '0';
        }

        cur ++;
    }
}

void initialize_board(board *b) {
    for (int i = 0; i < NUM_PIECES; i++) {
        for (int j = 0; j < NUM_SIDES; j++) {
            b->bitboards[i][j] = (bitboard){.piece = i, .side = j, .value = 0};
        }
    }
}
