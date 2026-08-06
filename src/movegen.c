/* File: movegen.c
 * ----------------
 * For further comments, see "movegen.h".
 */

#include <stddef.h>
#include <stdio.h>
#include "movegen.h"

/* FUNCTION PROTOTYPES */
static bool square_attacked(board *b, int sq, side s);
static void push_move(move_t *move_arr, size_t *index, int from_sq, int to_sq, move_flag flag);
static void push_promotions(move_t *move_arr, size_t *index, int from_sq, int to_sq, bool capture);
static void push_bb(move_t *move_arr, size_t *index, int from_sq, bitboard to_bb, move_flag flag);
static void push_attack_bb(move_t *move_arr, size_t *index, board *b, int from_sq, bitboard attack_bb);
static void generate_pawn_push(move_t *move_arr, size_t *index, board *b);
static void generate_pawn_dblpush(move_t *move_arr, size_t *index, board *b);
static void generate_pawn_attacks(move_t *move_arr, size_t *index, board *b);
static void generate_knight_moves(move_t *move_arr, size_t *index, board *b);
static void generate_king_moves(move_t *move_arr, size_t *index, board *b);
static bitboard get_slider_attack_bb(board *b, int from_sq, magic magic_table[]);
static void generate_rook_moves(move_t *move_arr, size_t *index, board *b);
static void generate_bishop_moves(move_t *move_arr, size_t *index, board *b);
static void generate_queen_moves(move_t *move_arr, size_t *index, board *b);

void generate_moves(move_t *move_arr, size_t *length, board *board) {
    size_t index = 0;

    generate_pawn_dblpush(move_arr, &index, board);
    generate_pawn_push(move_arr, &index, board);
    generate_pawn_attacks(move_arr, &index, board);
    generate_knight_moves(move_arr, &index, board);
    generate_king_moves(move_arr, &index, board);
    generate_rook_moves(move_arr, &index, board);
    generate_bishop_moves(move_arr, &index, board);
    generate_queen_moves(move_arr, &index, board);
    
    *length = index;
}

bool is_in_check(board *b, side s) {
    bitboard king_bb = b->piece_bbs[KING][s];
    int king_sq = bit_scan_forward(king_bb);

    return square_attacked(b, king_sq, s);
}

void print_move_list(move_t *move_arr, size_t length) {
    const char* flag_names[16] = {
        "QUIET",
        "DOUBLE_PAWN_PUSH",
        "KING_CASTLE",
        "QUEEN_CASTLE",
        "CAPTURE",
        "EP_CAPTURE",
        "",
        "",
        "KNIGHT_PROMO",
        "BISHOP_PROMO",
        "ROOK_PROMO",
        "QUEEN_PROMO",
        "KNIGHT_PROMO_CAPTURE",
        "BISHOP_PROMO_CAPTURE",
        "ROOK_PROMO_CAPTURE",
        "QUEEN_PROMO_CAPTURE"
    };
    
    for (size_t i = 0; i < length; i++) {
        move_t cur = move_arr[i];

        printf("Index %lu: FROM = %d, TO = %d, FLAG = %s\n", i, get_from(cur), get_to(cur), flag_names[get_flag(cur)]);
    }
}

/* Helper function to detect whether a square on a particular board position is being attacked
 * from the opposite side of s.
 */
static bool square_attacked(board *b, int sq, side s) {
    side opp_s = s ^ 1;

    if (pawn_attacks[s][sq] & b->piece_bbs[PAWN][opp_s]) return true;
    if (knight_attacks[sq] & b->piece_bbs[KNIGHT][opp_s]) return true;
    if (king_attacks[sq] & b->piece_bbs[KING][opp_s]) return true;
    if (get_slider_attack_bb(b, sq, rook_magics) & (b->piece_bbs[ROOK][opp_s] | b->piece_bbs[QUEEN][opp_s])) return true;
    if (get_slider_attack_bb(b, sq, bishop_magics) & (b->piece_bbs[BISHOP][opp_s] | b->piece_bbs[QUEEN][opp_s])) return true;
    
    return false;
}

/* Helper function to push a move to the move array. */
static void push_move(move_t *move_arr, size_t *index, int from_sq, int to_sq, move_flag flag) {
    move_arr[*index] = encode_move(from_sq, to_sq, flag);
    (*index) ++;
}

/* Helper function to push all promoting moves to the move array. The bool `capture` represents
 * whether the promoting moves are also captures.
 */
static void push_promotions(move_t *move_arr, size_t *index, int from_sq, int to_sq, bool capture) {
    move_flag flag_base = capture ? KNIGHT_PROMO_CAPTURE : KNIGHT_PROMO;
    
    for (int p = 0; p < NUM_PROMOS; p++) {
        push_move(move_arr, index, from_sq, to_sq, flag_base + p);
    }
}

/* Helper function to push all moves from a to_bb to the move array for a single from_sq.
 */
static void push_bb(move_t *move_arr, size_t *index, int from_sq, bitboard to_bb, move_flag flag) {
    while (to_bb) {
        int to_sq = bit_scan_forward(to_bb);
        push_move(move_arr, index, from_sq, to_sq, flag);
        to_bb &= to_bb - 1;
    }
}

/* Helper function that takes an attack bitboard and splits it into capture and quiet bitboards
 * to push into the move array. Should not be used by pawns in which from_sqs are computed on the fly.
 */
static void push_attack_bb(move_t *move_arr, size_t *index, board *b, int from_sq, bitboard attack_bb) {
    side s = b->play_side;
    
    attack_bb &= ~(b->occupied_bbs[s]);

    bitboard capture_bb = attack_bb & b->occupied_bbs[s == WHITE];
    bitboard quiet_bb = attack_bb ^ capture_bb;

    push_bb(move_arr, index, from_sq, capture_bb, CAPTURE);
    push_bb(move_arr, index, from_sq, quiet_bb, QUIET);
}

/* Helper function for outputting a generalized pawn push bitboard regardless of side.
 */
static inline bitboard get_pawnpush_bb(bitboard pawn_bb, side s) {
    return pawn_bb << VERT_SHIFT >> (s * 2 * VERT_SHIFT);
}

/* Helper function that populates the move array with single pawn pushes. */
static void generate_pawn_push(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard empty_bb = ~(b->occupied_bbs[WHITE] | b->occupied_bbs[BLACK]);
    
    bitboard to_bb = get_pawnpush_bb(b->piece_bbs[PAWN][s], s) & empty_bb;
    
    bitboard pawn_promote_bb = to_bb & (RANK_1 | RANK_8);
    bitboard pawn_push_bb = to_bb ^ pawn_promote_bb;

    while (pawn_promote_bb) {
        int to_sq = bit_scan_forward(pawn_promote_bb);
        int from_sq = (s == WHITE) ? to_sq - VERT_SHIFT : to_sq + VERT_SHIFT;

        push_promotions(move_arr, index, from_sq, to_sq, false);
        pawn_promote_bb &= pawn_promote_bb - 1;
    }
    
    while (pawn_push_bb) {
        int to_sq = bit_scan_forward(pawn_push_bb);
        int from_sq = (s == WHITE) ? to_sq - VERT_SHIFT : to_sq + VERT_SHIFT;

        push_move(move_arr, index, from_sq, to_sq, QUIET);
        pawn_push_bb &= pawn_push_bb - 1;
    }
}

/* Helper function that populates the move array wih double pawn pushes. */
static void generate_pawn_dblpush(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard empty_bb = ~(b->occupied_bbs[WHITE] | b->occupied_bbs[BLACK]);
    
    bitboard single_push_bb = get_pawnpush_bb(b->piece_bbs[PAWN][s], s) & empty_bb;
    bitboard double_push_bb = get_pawnpush_bb(single_push_bb, s) & empty_bb;

    double_push_bb &= (s == WHITE) ? RANK_4 : RANK_5;

    while (double_push_bb) {
        int to_sq = bit_scan_forward(double_push_bb);
        int from_sq = to_sq - (VERT_SHIFT << 1) + (s * (VERT_SHIFT << 2));

        push_move(move_arr, index, from_sq, to_sq, DOUBLE_PAWN_PUSH);
        double_push_bb &= double_push_bb - 1;
    }
}

/* Helper function that populates the move array with pawn attacks. */
static void generate_pawn_attacks(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard ep_bb = (b->ep_square != NO_EN_PASSANT) ? 1ULL << b->ep_square : 0;

    bitboard pawn_bb = b->piece_bbs[PAWN][s];

    while (pawn_bb) {
        int from_sq = bit_scan_forward(pawn_bb);
        bitboard to_bb = pawn_attacks[s][from_sq];
        to_bb &= (b->occupied_bbs[s == WHITE] | ep_bb);

        bool promote = to_bb & (RANK_1 | RANK_8);

        while (to_bb) {
            int to_sq = bit_scan_forward(to_bb);

            if (promote) {
                push_promotions(move_arr, index, from_sq, to_sq, true);
            } else {
                move_flag flag = (to_sq == b->ep_square) ? EP_CAPTURE : CAPTURE;
                push_move(move_arr, index, from_sq, to_sq, flag);
            }

            to_bb &= to_bb - 1;
        }
        
        pawn_bb &= pawn_bb - 1;
    }
}

/* Helper function that populates the move array with knight moves. */
static void generate_knight_moves(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard knight_bb = b->piece_bbs[KNIGHT][s];

    while (knight_bb) {
        int from_sq = bit_scan_forward(knight_bb);
        bitboard attack_bb = knight_attacks[from_sq];
        push_attack_bb(move_arr, index, b, from_sq, attack_bb);
        knight_bb &= knight_bb - 1;
    }
}

/* Helper function that populates the move array with king moves.
 * Assumes that there is only one king per side.
 */
static void generate_king_moves(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard king_bb = b->piece_bbs[KING][s];

    int from_sq = bit_scan_forward(king_bb);
    bitboard attack_bb = king_attacks[from_sq];
    push_attack_bb(move_arr, index, b, from_sq, attack_bb);

    // Castling
    if (is_in_check(b, s)) {
        return;
    }
    
    bool can_king_castle = b->castling & CASTLE_ARR_START >> (s * 2);
    bool can_queen_castle = b->castling & CASTLE_ARR_START >> (s * 2) >> 1;

    bitboard empty_bb = ~(b->occupied_bbs[WHITE] | b->occupied_bbs[BLACK]);
    bitboard king_castle_path = KING_CASTLE_PATH;
    bitboard queen_castle_path = QUEEN_CASTLE_PATH;

    if (s) {
        int shift_amt = (SIDE_LEN - 1) * VERT_SHIFT;
        king_castle_path <<= shift_amt;
        queen_castle_path <<= shift_amt;
    }
        
    if (can_king_castle &&
        (king_castle_path & empty_bb) == king_castle_path &&
        !square_attacked(b, from_sq + 1, s)) {
        push_move(move_arr, index, from_sq, from_sq + 2, KING_CASTLE);
    }

    if (can_queen_castle &&
        (queen_castle_path & empty_bb) == queen_castle_path &&
        !square_attacked(b, from_sq - 1, s)) {
        push_move(move_arr, index, from_sq, from_sq - 2, QUEEN_CASTLE);
    }
}

/* Helper function that lookups the slider attack bitboard with a given square
 * and magic table.
 */
static bitboard get_slider_attack_bb(board *b, int from_sq, magic magic_table[]) {
    magic cur_magic = magic_table[from_sq];
    bitboard blockers_bb = (b->occupied_bbs[WHITE] | b->occupied_bbs[BLACK]) & cur_magic.occ_mask;
    uint64_t table_index = (blockers_bb * cur_magic.magic_num) >> cur_magic.shift;

    return cur_magic.attack_table[table_index];
}

/* Helper function that populates the move array with rook moves. */
static void generate_rook_moves(move_t *move_arr, size_t *index, board *b) {
    bitboard rook_bb = b->piece_bbs[ROOK][b->play_side];

    while (rook_bb) {
        int from_sq = bit_scan_forward(rook_bb);
        bitboard attack_bb = get_slider_attack_bb(b, from_sq, rook_magics);

        push_attack_bb(move_arr, index, b, from_sq, attack_bb);
        rook_bb &= rook_bb - 1;
    }
}

/* Helper function that populates the move array with bishop moves. */
static void generate_bishop_moves(move_t *move_arr, size_t *index, board *b) {
    bitboard bishop_bb = b->piece_bbs[BISHOP][b->play_side];

    while (bishop_bb) {
        int from_sq = bit_scan_forward(bishop_bb);
        bitboard attack_bb = get_slider_attack_bb(b, from_sq, bishop_magics);

        push_attack_bb(move_arr, index, b, from_sq, attack_bb);
        bishop_bb &= bishop_bb - 1;
    }
}

/* Helper function that populates the move array with queen moves. */
static void generate_queen_moves(move_t *move_arr, size_t *index, board *b) {
    bitboard queen_bb = b->piece_bbs[QUEEN][b->play_side];
    
    while (queen_bb) {
        int from_sq = bit_scan_forward(queen_bb);
        bitboard attack_bb = get_slider_attack_bb(b, from_sq, rook_magics) | get_slider_attack_bb(b, from_sq, bishop_magics);

        push_attack_bb(move_arr, index, b, from_sq, attack_bb);
        queen_bb &= queen_bb - 1;
    }
}
