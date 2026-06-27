/* File: move_make.c
 * ------------------
 * For further comments, see "move_make.h".
 */

#include <stddef.h>
#include "move_make.h"

/* Used for unmaking moves. */
static undo_move_t move_stack[MAX_PLY];
static size_t move_stack_index = 0;

// Helper function of `make_move` to update a board's move counters.
static void update_move_counters(board *b, piece_t move_p, side move_s, move_flag move_f) {
    move_stack[move_stack_index].halfmove_clock = b->halfmove_clock;
    
    if (move_p == PAWN || move_f & CAPTURE_FLAG) {
        b->halfmove_clock = 0;
    } else {
        b->halfmove_clock ++;
    }

    if (move_s == BLACK) {
        b->fullmove_counter ++;
    }
}

// Helper function to update a board's play_side
static void update_playside(board *b, side move_s) {
    if (move_s == WHITE) {
        b->play_side = BLACK;
    } else {
        b->play_side = WHITE;
    }
}

// Helper function of `make_move` to update a board's en passant square.
static void update_ep(board *b, int to_sq, side move_s, move_flag move_f) {
    move_stack[move_stack_index].ep_square = b->ep_square;
    
    if (move_f == DOUBLE_PAWN_PUSH) {
        b->ep_square = (to_sq - VERT_SHIFT) + (move_s << VERT_SHIFT_POWER << 1);
    } else {
        b->ep_square = NO_EN_PASSANT;
    }
}

// Helper function of `make_move` to update a board's castling bitarray.
static void update_castling(board *b, int from_sq, piece_t move_p, side move_s) {
    move_stack[move_stack_index].castling = b->castling;

    if (move_p == KING) {
        b->castling &= ~(CASTLE_ARR_START >> (2 * move_s));
        b->castling &= ~(CASTLE_ARR_START >> 1 >> (2 * move_s));
    } else if (move_p == ROOK &&
               ((unsigned int)from_sq == KING_ROOK_START + (move_s * (SIDE_LEN - 1) * SIDE_LEN) ||
                (unsigned int)from_sq == QUEEN_ROOK_START + (move_s * (SIDE_LEN - 1) * SIDE_LEN))) {
        b->castling &= ~(CASTLE_ARR_START >>
                         ((unsigned int)from_sq == QUEEN_ROOK_START + (move_s * (SIDE_LEN - 1) * SIDE_LEN)) >>
                         (2 * move_s));
    }
}

// Helper function of `make_move` to remove a captured piece from its bitboard.
static void remove_captured_piece(board *b, int to_sq, side move_s, move_flag f) {
    int capture_sq;

    if (f == EP_CAPTURE) {
        capture_sq = (to_sq - VERT_SHIFT) + (move_s << VERT_SHIFT_POWER << 1);
    } else {
        capture_sq = to_sq;
    }
        
    side capture_side;
    piece_t capture_piece;
    bitboard *capture_bb = get_bitboard_from_square(b, capture_sq, &capture_piece, &capture_side);

    clear_bit(capture_bb, capture_sq);
    clear_bit(&(b->occupied_bbs[capture_side]), capture_sq);

    // Handle castling rights if captured piece is a rook
    if (capture_piece == ROOK) {
        b->castling &= ~(CASTLE_ARR_START >>
                         ((unsigned int)to_sq == QUEEN_ROOK_START + (capture_side * (SIDE_LEN - 1) * SIDE_LEN)) >>
                         (2 * capture_side));
    }

    move_stack[move_stack_index].captured_piece = capture_piece;
}

void make_move(board *b, move_t move) {
    int from_sq = get_from(move);
    int to_sq = get_to(move);
    move_flag move_f = get_flag(move);

    piece_t move_p;
    side move_s;
    bitboard *from_bb = get_bitboard_from_square(b, from_sq, &move_p, &move_s);
    bitboard *to_bb;

    // Update board struct
    update_move_counters(b, move_p, move_s, move_f);
    update_playside(b, move_s);
    update_ep(b, to_sq, move_s, move_f);
    update_castling(b, from_sq, move_p, move_s);

    // Handle capturing
    if (move_f & CAPTURE_FLAG) {
        remove_captured_piece(b, to_sq, move_s, move_f);
    }

    // Handle promotion
    if (move_f & PROMO_FLAG) {
        piece_t promo_piece = (move_f & SPECIAL_FLAG) + KNIGHT; 
        to_bb = &(b->piece_bbs[promo_piece][move_s]);
    } else {
        to_bb = from_bb;
    }

    // Handle castling
    if (move_f == KING_CASTLE || move_f == QUEEN_CASTLE) {
        int rook_from = (move_s * SIDE_LEN * (SIDE_LEN - 1)) +
            (move_f ^ QUEEN_CASTLE) * (SIDE_LEN - 1);
        
        int rook_to = (move_s * SIDE_LEN * (SIDE_LEN - 1)) +
            (move_f ^ KING_CASTLE) * ROOK_QUEEN_CASTLE +
            (move_f ^ QUEEN_CASTLE) * ROOK_KING_CASTLE;
        
        clear_bit(&(b->piece_bbs[ROOK][move_s]), rook_from);
        clear_bit(&(b->occupied_bbs[move_s]), rook_from);
        set_bit(&(b->piece_bbs[ROOK][move_s]), rook_to);
        set_bit(&(b->occupied_bbs[move_s]), rook_to);
    }

    // Move piece
    clear_bit(from_bb, from_sq);
    clear_bit(&(b->occupied_bbs[move_s]), from_sq);
    set_bit(to_bb, to_sq);
    set_bit(&(b->occupied_bbs[move_s]), to_sq);

    move_stack_index ++;
}

void unmake_move(board *b, move_t move) {
    int from_sq = get_from(move);
    int to_sq = get_to(move);
    move_flag move_f = get_flag(move);

    piece_t move_p;
    side move_s;
    bitboard *from_bb = get_bitboard_from_square(b, to_sq, &move_p, &move_s);
    bitboard *to_bb = from_bb;

    // Update board struct
    move_stack_index --;
    b->play_side = move_s;
    b->ep_square = move_stack[move_stack_index].ep_square;
    b->castling = move_stack[move_stack_index].castling;
    b->halfmove_clock = move_stack[move_stack_index].halfmove_clock;
    b->fullmove_counter -= move_s;

    // Handle capturing
    if (move_f & CAPTURE_FLAG) {
        int capture_sq;

        if (move_f == EP_CAPTURE) {
            capture_sq = (to_sq - VERT_SHIFT) + (move_s << VERT_SHIFT_POWER << 1);
        } else {
            capture_sq = to_sq;
        }

        piece_t capture_p = move_stack[move_stack_index].captured_piece;
        side capture_s = move_s == WHITE;
        bitboard *capture_bb = &(b->piece_bbs[capture_p][capture_s]);

        set_bit(capture_bb, capture_sq);
        set_bit(&(b->occupied_bbs[capture_s]), capture_sq);
    }

    // Handle promotion
    if (move_f & PROMO_FLAG) {
        from_bb = &(b->piece_bbs[PAWN][move_s]);
    }

    // Handle castling
    if (move_f == KING_CASTLE || move_f == QUEEN_CASTLE) {
        int rook_from = (move_s * SIDE_LEN * (SIDE_LEN - 1)) +
            (move_f ^ QUEEN_CASTLE) * (SIDE_LEN - 1);
        
        int rook_to = (move_s * SIDE_LEN * (SIDE_LEN - 1)) +
            (move_f ^ KING_CASTLE) * ROOK_QUEEN_CASTLE +
            (move_f ^ QUEEN_CASTLE) * ROOK_KING_CASTLE;
        
        set_bit(&(b->piece_bbs[ROOK][move_s]), rook_from);
        set_bit(&(b->occupied_bbs[move_s]), rook_from);
        clear_bit(&(b->piece_bbs[ROOK][move_s]), rook_to);
        clear_bit(&(b->occupied_bbs[move_s]), rook_to);
    }

    // Undo piece move
    set_bit(from_bb, from_sq);
    set_bit(&(b->occupied_bbs[move_s]), from_sq);
    clear_bit(to_bb, to_sq);
    clear_bit(&(b->occupied_bbs[move_s]), to_sq);
}
