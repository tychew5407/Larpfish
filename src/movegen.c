/* File: movegen.c
 * ----------------
 * For further comments, see "movegen.h".
 */

#include <stddef.h>
#include <stdio.h>
#include "movegen.h"

static const ray rays[RAY_DIRS] = {
    {NORTH, VERT_SHIFT, false, ~0},
    {NORTHEAST, VERT_SHIFT + 1, false, ~FILE_A},
    {EAST, 1, false, ~FILE_A},
    {SOUTHEAST, VERT_SHIFT - 1, true, ~FILE_A},
    {SOUTH, VERT_SHIFT, true, ~0},
    {SOUTHWEST, VERT_SHIFT + 1, true, ~FILE_H},
    {WEST, 1, true, ~FILE_H},
    {NORTHWEST, VERT_SHIFT - 1, false, ~FILE_H}
};

// To index into rays array, with -1 as a sentinel value to stop.
static const int slider_ray_indices[NUM_SLIDERS][RAY_DIRS + 1] = {
    {1, 3, 5, 7, -1, 0, 0, 0, 0},  // BISHOP
    {0, 2, 4, 6, -1, 0, 0, 0, 0},  // ROOK
    {0, 1, 2, 3, 4, 5, 6, 7, -1}   // QUEEN
};

static bitboard pawn_attacks[NUM_SIDES][NUM_SQUARES];
static bitboard knight_attacks[NUM_SQUARES] = {0};
static bitboard king_attacks[NUM_SQUARES] = {0};
static bitboard ray_attacks[NUM_SQUARES][RAY_DIRS] = {0};

/* Helper function to precompute the pawn attack table. */
static void init_pawn_attacks() {
    for (int sq = 0; sq < NUM_SQUARES; sq ++) {
        bitboard sq_bb = 1ULL << sq;
        
        pawn_attacks[WHITE][sq] = (sq_bb & ~FILE_A) << VERT_SHIFT >> 1;
        pawn_attacks[WHITE][sq] |= (sq_bb & ~FILE_H) << VERT_SHIFT << 1;
        pawn_attacks[BLACK][sq] = (sq_bb & ~FILE_A) >> VERT_SHIFT >> 1;
        pawn_attacks[BLACK][sq] |= (sq_bb & ~FILE_H) >> VERT_SHIFT << 1;
    }
}

/* Helper function to precompute the knight attack table. */
static void init_knight_attacks() {
    for (int sq = 0; sq < NUM_SQUARES; sq ++) {
        bitboard sq_bb = 1ULL << sq;

        bitboard dir_bbs[KNIGHT_DIRS] = {
            (sq_bb & ~FILE_H) << (VERT_SHIFT << 1) << 1, // NNE
            (sq_bb & ~FILE_A) << (VERT_SHIFT << 1) >> 1, // NNW
            (sq_bb & ~FILE_G & ~FILE_H) << VERT_SHIFT << 2, // NEE
            (sq_bb & ~FILE_A & ~FILE_B) << VERT_SHIFT >> 2, // NWW
            (sq_bb & ~FILE_H) >> (VERT_SHIFT << 1) << 1, // SSE
            (sq_bb & ~FILE_A) >> (VERT_SHIFT << 1) >> 1, // SSW
            (sq_bb & ~FILE_G & ~FILE_H) >> VERT_SHIFT << 2, // SEE
            (sq_bb & ~FILE_A & ~FILE_B) >> VERT_SHIFT >> 2, // SWW
        };

        for (int i = 0; i < KNIGHT_DIRS; i++) {
            knight_attacks[sq] |= dir_bbs[i];
        }
    }
}

/* Helper function to precompute the king attack table. */
static void init_king_attacks() {
    for (int sq = 0; sq < NUM_SQUARES; sq ++) {
        bitboard sq_bb = 1ULL << sq;

        bitboard dir_bbs[KING_DIRS] = {
            (sq_bb & ~FILE_H) << VERT_SHIFT << 1, // NE
            (sq_bb & ~FILE_A) << VERT_SHIFT >> 1, // NW
            (sq_bb & ~FILE_H) >> VERT_SHIFT << 1, // SE
            (sq_bb & ~FILE_A) >> VERT_SHIFT >> 1, // SW
            sq_bb << VERT_SHIFT,                  // N
            sq_bb >> VERT_SHIFT,                  // S
            (sq_bb & ~FILE_H) << 1,               // E
            (sq_bb & ~FILE_A) >> 1,               // W
        };

        for (int i = 0; i < KING_DIRS; i++) {
            king_attacks[sq] |= dir_bbs[i];
        }
    }
}

/* Helper function to precompute ray attacks. */
static void init_ray_attacks() {
    for (int sq = 0; sq < NUM_SQUARES; sq ++) {
        for (int i = 0; i < RAY_DIRS; i++) {
            ray cur = rays[i];

            bitboard ray_bb = 1ULL << sq;
            
            if (cur.negative) {
                ray_bb >>= cur.shift_amount;
            } else {
                ray_bb <<= cur.shift_amount;
            }

            while (ray_bb & cur.wrap_check) {
                ray_attacks[sq][cur.dir] |= ray_bb;

                if (cur.negative) {
                    ray_bb >>= cur.shift_amount;
                } else {
                    ray_bb <<= cur.shift_amount;
                }
            }
        }
    }
}

static bitboard get_pawnpush_bb(bitboard pawn_bb, side s) {
    return pawn_bb << VERT_SHIFT >> (s << VERT_SHIFT_POWER << 1);
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

/* Helper function to push all moves from a to_bb to the move array. For all pieces but pawns
 * (since pawns have to consider additional flags).
 */
static void push_bb(move_t *move_arr, size_t *index, board *b, int from_sq, bitboard to_bb) {
    side s = b->play_side;
    to_bb &= ~(b->occupied_bbs[s]);

    while (to_bb) {
        int to_sq = bit_scan(to_bb);
        move_flag flag = ((1ULL << to_sq) & b->occupied_bbs[s == WHITE]) ? CAPTURE : QUIET;
        push_move(move_arr, index, from_sq, to_sq, flag);
        to_bb ^= (1ULL << to_sq);
    }
}

/* Helper function that populates the move array with single pawn pushes. */
static void generate_pawn_push(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard empty_bb = (~(b->occupied_bbs[WHITE])) & (~(b->occupied_bbs[BLACK]));
    
    bitboard to_bb = get_pawnpush_bb(b->piece_bbs[PAWN][s], s);
    to_bb &= empty_bb;

    while (to_bb) {
        int to_sq = bit_scan(to_bb);
        int from_sq = to_sq - VERT_SHIFT + (s * (VERT_SHIFT << 1));
        
        if ((1ULL << to_sq) & (RANK_1 | RANK_8)) {
            push_promotions(move_arr, index, from_sq, to_sq, false);
        } else {
            push_move(move_arr, index, from_sq, to_sq, QUIET);
        }
            
        to_bb ^= (1ULL << to_sq);
    }
}

/* Helper function that populates the move array wih double pawn pushes. */
static void generate_pawn_dblpush(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard empty = (~(b->occupied_bbs[WHITE])) & (~(b->occupied_bbs[BLACK]));
    
    bitboard to_bb = get_pawnpush_bb(b->piece_bbs[PAWN][s], s);
    to_bb &= empty;
    to_bb = get_pawnpush_bb(to_bb, s);
    to_bb &= empty & ((s * RANK_5) | ((s == WHITE) * RANK_4));

    while (to_bb) {
        int to_sq = bit_scan(to_bb);
        int from_sq = to_sq - (VERT_SHIFT << 1) + (s * (VERT_SHIFT << 2));

        push_move(move_arr, index, from_sq, to_sq, DOUBLE_PAWN_PUSH);
        
        to_bb ^= (1ULL << to_sq);
    }
}

/* Helper function that populates the move array with pawn attacks. */
static void generate_pawn_attacks(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard ep_bb = (b->ep_square != NO_EN_PASSANT) ? 1ULL << b->ep_square : 0;

    bitboard pawn_bb = b->piece_bbs[PAWN][s];

    while (pawn_bb) {
        int from_sq = bit_scan(pawn_bb);
        bitboard to_bb = pawn_attacks[s][from_sq];
        to_bb &= (b->occupied_bbs[s == WHITE] | ep_bb);

        while (to_bb) {
            int to_sq = bit_scan(to_bb);

            if ((1ULL << to_sq) & (RANK_1 | RANK_8)) {
                push_promotions(move_arr, index, from_sq, to_sq, true);
            } else {
                move_flag flag = (to_sq == b->ep_square) ? EP_CAPTURE : CAPTURE;
                push_move(move_arr, index, from_sq, to_sq, flag);
            }
            
            to_bb ^= (1ULL << to_sq);
        }
        
        pawn_bb ^= (1ULL << from_sq);
    }
}

/* Helper function that populates the move array with knight moves. */
static void generate_knight_moves(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard knight_bb = b->piece_bbs[KNIGHT][s];

    while (knight_bb) {
        int from_sq = bit_scan(knight_bb);
        push_bb(move_arr, index, b, from_sq, knight_attacks[from_sq]);
        knight_bb ^= (1ULL << from_sq);
    }
}

/* Helper function that populates the move array with king moves. */
static void generate_king_moves(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard king_bb = b->piece_bbs[KING][s];

    while (king_bb) {
        int from_sq = bit_scan(king_bb);
        push_bb(move_arr, index, b, from_sq, king_attacks[from_sq]);

        // Castling
        bitboard empty = (~(b->occupied_bbs[WHITE])) & (~(b->occupied_bbs[BLACK]));
        bitboard side_shift = s * VERT_SHIFT * (SIDE_LEN - 1); // amount to shift path based on side
        
        bitboard king_castle_path = KING_CASTLE_PATH << side_shift;
        bitboard queen_castle_path = QUEEN_CASTLE_PATH << side_shift;
        
        if (b->castling & CASTLE_ARR_START >> (s << 1) &&
            (king_castle_path & empty) == king_castle_path) {
            push_move(move_arr, index, from_sq, from_sq + 2, KING_CASTLE);
        }

        if (b->castling & CASTLE_ARR_START >> (s << 1) >> 1 &&
            (queen_castle_path & empty) == queen_castle_path) {
            push_move(move_arr, index, from_sq, from_sq - 2, QUEEN_CASTLE);
        }
        
        king_bb ^= (1ULL << from_sq);
    }
}

/* Helper function that populates the move array with moves of any specified slider piece. */
static void generate_slider_moves(move_t *move_arr, size_t *index, board *b, piece_t p) {
    side s = b->play_side;
    bitboard piece_bb = b->piece_bbs[p][s];

    while (piece_bb) {
        int from_sq = bit_scan(piece_bb);

        for (int i = 0; i < RAY_DIRS + 1; i++) {
            int ray_i = slider_ray_indices[p - BISHOP][i];
            if (ray_i == -1) break;

            bitboard to_bb = ray_attacks[from_sq][ray_i];
            bitboard blockers_bb = to_bb & (b->occupied_bbs[WHITE] | b->occupied_bbs[BLACK]);

            if (blockers_bb) {
                int block_sq = (rays[ray_i].negative) ? bit_scan_reverse(blockers_bb) : bit_scan(blockers_bb);
                to_bb ^= ray_attacks[block_sq][ray_i];
            }

            push_bb(move_arr, index, b, from_sq, to_bb);
        }

        piece_bb ^= (1ULL << from_sq);
    }
}

void init_attack_tables() {
    init_pawn_attacks();
    init_knight_attacks();
    init_king_attacks();
    init_ray_attacks();
}

void generate_moves(move_t *move_arr, size_t *length, board *board) {
    size_t index = 0;

    generate_pawn_dblpush(move_arr, &index, board);
    generate_pawn_push(move_arr, &index, board);
    generate_pawn_attacks(move_arr, &index, board);
    generate_knight_moves(move_arr, &index, board);
    generate_king_moves(move_arr, &index, board);
    generate_slider_moves(move_arr, &index, board, BISHOP);
    generate_slider_moves(move_arr, &index, board, ROOK);
    generate_slider_moves(move_arr, &index, board, QUEEN);
    
    *length = index;
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
