/* File: move_make.c
 * ------------------
 * For further comments, see "move_make.h".
 */

#include <stddef.h>
#include "bitboard.h"
#include "move_make.h"

/* Used for unmaking moves. */
static undo_move_t move_stack[MAX_PLY];
static size_t move_stack_index = 0;

/* Helper function of `make_move` to update a board's move counters. This helper function
 * must be called before the other helper functions to properly update the game history, as
 * it also initializes the new element in `game_history`.
 */
static void update_move_counters(board *b, zobrist_board *game_history, piece_t move_p, side move_s, move_flag move_f) {
    move_stack[move_stack_index].halfmove_clock = b->halfmove_clock;
    
    if (move_p == PAWN || move_f & CAPTURE_FLAG) {
        b->halfmove_clock = 0;
    } else {
        b->halfmove_clock ++;
    }

    if (move_s == BLACK) {
        b->fullmove_counter ++;
    }

    // Initialize new game history elem
    move_stack[move_stack_index].zobrist_pos = game_history[b->halfmove_clock];
    game_history[b->halfmove_clock] = game_history[move_stack[move_stack_index].halfmove_clock];
}

// Helper function to update a board's play_side
static void update_playside(board *b, zobrist_board *game_history, side move_s) {
    b->play_side = (move_s == WHITE);
    toggle_zobrist_turn(&game_history[b->halfmove_clock]);
}

// Helper function of `make_move` to update a board's en passant square.
static void update_ep(board *b, zobrist_board *game_history, int to_sq, side move_s, move_flag move_f) {
    move_stack[move_stack_index].ep_square = b->ep_square;
    
    if (b->ep_square != NO_EN_PASSANT) {
        toggle_zobrist_ep(&game_history[b->halfmove_clock], get_file(b->ep_square));
    }
    
    
    if (move_f == DOUBLE_PAWN_PUSH) {
        b->ep_square = (to_sq - VERT_SHIFT) + (move_s << VERT_SHIFT_POWER << 1);
        toggle_zobrist_ep(&game_history[b->halfmove_clock], get_file(b->ep_square));
    } else {
        b->ep_square = NO_EN_PASSANT;
    }
}

// Helper function of `make_move` to update a board's castling bitarray.
static void update_castling(board *b, zobrist_board *game_history, int from_sq, piece_t move_p, side move_s) {
    move_stack[move_stack_index].castling = b->castling;

    if (move_p == KING) {
        toggle_zobrist_castling(&game_history[b->halfmove_clock], b->castling);
        b->castling &= ~(CASTLE_ARR_START >> (2 * move_s));
        b->castling &= ~(CASTLE_ARR_START >> 1 >> (2 * move_s));
        toggle_zobrist_castling(&game_history[b->halfmove_clock], b->castling);
    } else if (move_p == ROOK &&
               ((unsigned int)from_sq == KING_ROOK_START + (move_s * (SIDE_LEN - 1) * SIDE_LEN) ||
                (unsigned int)from_sq == QUEEN_ROOK_START + (move_s * (SIDE_LEN - 1) * SIDE_LEN))) {
        toggle_zobrist_castling(&game_history[b->halfmove_clock], b->castling);
        b->castling &= ~(CASTLE_ARR_START >>
                         ((unsigned int)from_sq == QUEEN_ROOK_START + (move_s * (SIDE_LEN - 1) * SIDE_LEN)) >>
                         (2 * move_s));
        toggle_zobrist_castling(&game_history[b->halfmove_clock], b->castling);
    }
}

// Helper function of `make_move` to remove a captured piece from its bitboard.
static void remove_captured_piece(board *b, zobrist_board *game_history, int to_sq, side move_s, move_flag f) {
    int capture_sq;

    if (f == EP_CAPTURE) {
        capture_sq = (to_sq - VERT_SHIFT) + (move_s << VERT_SHIFT_POWER << 1);
    } else {
        capture_sq = to_sq;
    }

    piece_t capture_piece = piece_on(b, capture_sq);
    side capture_side = side_on(b, capture_sq);
    bitboard *capture_bb = get_bitboard_from_square(b, capture_sq);

    clear_bit(capture_bb, capture_sq);
    clear_bit(&(b->occupied_bbs[capture_side]), capture_sq);
    b->piece_mailbox[capture_sq] = NO_PIECE;
    toggle_zobrist_piece(&game_history[b->halfmove_clock], capture_piece, capture_side, capture_sq);

    // Handle castling rights if captured piece is a rook
    if (capture_piece == ROOK &&
        ((unsigned int)to_sq == KING_ROOK_START + (capture_side * (SIDE_LEN - 1) * SIDE_LEN) ||
         (unsigned int)to_sq == QUEEN_ROOK_START + (capture_side * (SIDE_LEN - 1) * SIDE_LEN))) {
        toggle_zobrist_castling(&game_history[b->halfmove_clock], b->castling);
        b->castling &= ~(CASTLE_ARR_START >>
                         ((unsigned int)to_sq == QUEEN_ROOK_START + (capture_side * (SIDE_LEN - 1) * SIDE_LEN)) >>
                         (2 * capture_side));
        toggle_zobrist_castling(&game_history[b->halfmove_clock], b->castling);
    }

    move_stack[move_stack_index].captured_piece = capture_piece;
}

void make_move(board *b, zobrist_board *game_history, move_t move) {
    int from_sq = get_from(move);
    int to_sq = get_to(move);
    move_flag move_f = get_flag(move);
    
    side move_s = side_on(b, from_sq);
    piece_t from_p = piece_on(b, from_sq);
    bitboard *from_bb = get_bitboard_from_square(b, from_sq);

    piece_t to_p = (move_f & PROMO_FLAG) ? (piece_t)((move_f & SPECIAL_FLAG) + KNIGHT) : from_p;
    bitboard *to_bb = &(b->piece_bbs[to_p][move_s]);

    // Update board struct and game history
    update_move_counters(b, game_history, from_p, move_s, move_f);
    update_playside(b, game_history, move_s);
    update_ep(b, game_history, to_sq, move_s, move_f);
    update_castling(b, game_history, from_sq, from_p, move_s);

    // Handle capturing
    if (move_f & CAPTURE_FLAG) {
        remove_captured_piece(b, game_history, to_sq, move_s, move_f);
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
        b->piece_mailbox[rook_from] = NO_PIECE;
        toggle_zobrist_piece(&game_history[b->halfmove_clock], ROOK, move_s, rook_from);
        
        set_bit(&(b->piece_bbs[ROOK][move_s]), rook_to);
        set_bit(&(b->occupied_bbs[move_s]), rook_to);
        b->piece_mailbox[rook_to] = ROOK;
        b->side_mailbox[rook_to] = move_s;
        toggle_zobrist_piece(&game_history[b->halfmove_clock], ROOK, move_s, rook_to);
    }

    // Move piece
    clear_bit(from_bb, from_sq);
    clear_bit(&(b->occupied_bbs[move_s]), from_sq);
    b->piece_mailbox[from_sq] = NO_PIECE;
    toggle_zobrist_piece(&game_history[b->halfmove_clock], from_p, move_s, from_sq);
    
    set_bit(to_bb, to_sq);
    set_bit(&(b->occupied_bbs[move_s]), to_sq);
    b->piece_mailbox[to_sq] = to_p;
    b->side_mailbox[to_sq] = move_s;
    toggle_zobrist_piece(&game_history[b->halfmove_clock], to_p, move_s, to_sq);

    move_stack_index ++;
}

void unmake_move(board *b, zobrist_board *game_history, move_t move) {
    int from_sq = get_from(move);
    int to_sq = get_to(move);
    move_flag move_f = get_flag(move);
    
    side move_s = side_on(b, to_sq);
    piece_t from_p = piece_on(b, to_sq);
    bitboard *from_bb = get_bitboard_from_square(b, to_sq);

    bitboard *to_bb = from_bb;

    // Update board struct + game history
    move_stack_index --;
    game_history[b->halfmove_clock] = move_stack[move_stack_index].zobrist_pos;
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
        b->piece_mailbox[capture_sq] = capture_p;
        b->side_mailbox[capture_sq] = capture_s;
    }

    // Handle promotion
    if (move_f & PROMO_FLAG) {
        from_bb = &(b->piece_bbs[PAWN][move_s]);
        from_p = PAWN;
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
        b->piece_mailbox[rook_from] = ROOK;
        b->side_mailbox[rook_from] = move_s;
        clear_bit(&(b->piece_bbs[ROOK][move_s]), rook_to);
        clear_bit(&(b->occupied_bbs[move_s]), rook_to);
        b->piece_mailbox[rook_to] = NO_PIECE;
    }

    // Undo piece move
    set_bit(from_bb, from_sq);
    set_bit(&(b->occupied_bbs[move_s]), from_sq);
    b->piece_mailbox[from_sq] = from_p;
    b->side_mailbox[from_sq] = move_s;
    
    clear_bit(to_bb, to_sq);
    clear_bit(&(b->occupied_bbs[move_s]), to_sq);
    
    if (!(move_f & CAPTURE_FLAG) || move_f == EP_CAPTURE) {
        b->piece_mailbox[to_sq] = NO_PIECE;
    }
}

void empty_move_stack() {
    move_stack_index = 0;
}
