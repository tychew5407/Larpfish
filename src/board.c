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

void set_bit(bitboard *bb, int square) {
    *bb |= (1ULL << square);
}

void clear_bit(bitboard *bb, int square) {
    *bb &= ~(1ULL << square);
}

bool get_bit(bitboard bb, int square) {
    return (bb & (1ULL << square)) > 0;
}

int get_file(int square) {
    return square & (SIDE_LEN - 1);
}

int get_rank(int square) {
    return square >> SIDE_LEN_POWER;
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

bitboard get_occupied(board *b, side s) {
    bitboard result = 0;
    
    for (int i = 0; i < NUM_PIECES; i++) {
        result |= b->piece_bbs[i][s];
    }

    return result;
}

void generate_moves(move_t *move_arr[], board *board) {
    
}

void make_move(board *b, move_t move) {
    int from_sq = get_from(move);
    int to_sq = get_to(move);
    move_flag flag = get_flag(move);

    side move_side;
    bitboard *from_bb = get_bitboard_from_square(b, from_sq, NULL, &move_side);
    bitboard *to_bb;

    if (flag == DOUBLE_PAWN_PUSH) {
        b->ep_square = (to_sq - VERT_SHIFT) + (move_side << VERT_SHIFT_POWER << 1);
    } else {
        b->ep_square = NO_EN_PASSANT;
    }

    // TODO: push ep_square onto move stack for undoing moves
    
    if (flag & CAPTURE_FLAG) {
        int capture_sq;

        if (flag == EP_CAPTURE) {
            capture_sq = (to_sq - VERT_SHIFT) + (move_side << VERT_SHIFT_POWER << 1);
        } else {
            capture_sq = to_sq;
        }
        
        side capture_side;
        bitboard *capture_bb = get_bitboard_from_square(b, capture_sq, NULL, &capture_side);
        // TODO: push captured piece onto move stack for undoing moves

        clear_bit(capture_bb, capture_sq);
        clear_bit(&(b->occupied_bbs[capture_side]), capture_sq);
    }
    
    if (flag & PROMO_FLAG) {
        /* The special flag corresponds to pieces in order of the enum piece_t,
         * starting from KNIGHT.
         */
        
        piece_t promo_piece = (flag & SPECIAL_FLAG) + KNIGHT; 
        to_bb = &(b->piece_bbs[promo_piece][move_side]);
    } else {
        to_bb = from_bb;
    }

    clear_bit(from_bb, from_sq);
    clear_bit(&(b->occupied_bbs[move_side]), from_sq);
    set_bit(to_bb, to_sq);
    set_bit(&(b->occupied_bbs[move_side]), to_sq);
}

void unmake_move(board *b, move_t move) {
    
}
