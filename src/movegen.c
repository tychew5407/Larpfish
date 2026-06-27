/* File: movegen.c
 * ----------------
 * For further comments, see "movegen.h".
 */

#include <stddef.h>
#include <stdio.h>
#include "movegen.h"

/* PRECOMPUTED ATTACK TABLES/HELPERS */

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

/* HELPER FUNCTION PROTOTYPES */
static void init_pawn_attacks();
static void init_knight_attacks();
static void init_king_attacks();
static void init_ray_attacks();

static bool square_attacked(board *b, int sq, side s);

static void push_move(move_t *move_arr, size_t *index, int from_sq, int to_sq, move_flag flag);
static void push_promotions(move_t *move_arr, size_t *index, int from_sq, int to_sq, bool capture);
static void push_bb(move_t *move_arr, size_t *index, int from_sq, bitboard to_bb, move_flag flag);
static void push_attack_bb(move_t *move_arr, size_t *index, board *b, int from_sq, bitboard attack_bb);

static bitboard get_pawnpush_bb(bitboard pawn_bb, side s);
static void generate_pawn_push(move_t *move_arr, size_t *index, board *b);
static void generate_pawn_dblpush(move_t *move_arr, size_t *index, board *b);
static void generate_pawn_attacks(move_t *move_arr, size_t *index, board *b);

static void generate_knight_moves(move_t *move_arr, size_t *index, board *b);

static void generate_king_moves(move_t *move_arr, size_t *index, board *b);

static bitboard generate_ray_bitboard(board *b, ray_dir dir, int square);
static void generate_slider_moves(move_t *move_arr, size_t *index, board *b, piece_t p);

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

bool is_in_check(board *b, side s) {
    bitboard king_bb = b->piece_bbs[KING][s];
    int king_sq = bit_scan(king_bb);

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

/* Helper function to detect whether a square on a particular board position is being attacked
 * from the opposite side of s.
 */
static bool square_attacked(board *b, int sq, side s) {
    side opp_s = s ^ 1;

    if (pawn_attacks[s][sq] & b->piece_bbs[PAWN][opp_s]) return true;
    if (knight_attacks[sq] & b->piece_bbs[KNIGHT][opp_s]) return true;
    if (king_attacks[sq] & b->piece_bbs[KING][opp_s]) return true;

    bitboard rook_bb = 0;
    for (int i = 0; i < RAY_DIRS; i += 2) {
        rook_bb |= generate_ray_bitboard(b, (ray_dir)i, sq);
    }

    if (rook_bb & (b->piece_bbs[ROOK][opp_s] | b->piece_bbs[QUEEN][opp_s])) return true;

    bitboard bishop_bb = 0;
    for (int i = 1; i < RAY_DIRS; i += 2) {
        bishop_bb |= generate_ray_bitboard(b, (ray_dir)i, sq);
    }

    if (bishop_bb & (b->piece_bbs[BISHOP][opp_s] | b->piece_bbs[QUEEN][opp_s])) return true;
    
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
        int to_sq = bit_scan(to_bb);
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
static bitboard get_pawnpush_bb(bitboard pawn_bb, side s) {
    return pawn_bb << VERT_SHIFT >> (s << VERT_SHIFT_POWER << 1);
}

/* Helper function that populates the move array with single pawn pushes. */
static void generate_pawn_push(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard empty_bb = ~(b->occupied_bbs[WHITE] | b->occupied_bbs[BLACK]);
    
    bitboard to_bb = get_pawnpush_bb(b->piece_bbs[PAWN][s], s) & empty_bb;
    
    bitboard pawn_promote_bb = to_bb & (RANK_1 | RANK_8);
    bitboard pawn_push_bb = to_bb ^ pawn_promote_bb;

    while (pawn_promote_bb) {
        int to_sq = bit_scan(pawn_promote_bb);
        int from_sq = (s == WHITE) ? to_sq - VERT_SHIFT : to_sq + VERT_SHIFT;

        push_promotions(move_arr, index, from_sq, to_sq, false);
        pawn_promote_bb &= pawn_promote_bb - 1;
    }
    
    while (pawn_push_bb) {
        int to_sq = bit_scan(pawn_push_bb);
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
        int to_sq = bit_scan(double_push_bb);
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
        int from_sq = bit_scan(pawn_bb);
        bitboard to_bb = pawn_attacks[s][from_sq];
        to_bb &= (b->occupied_bbs[s == WHITE] | ep_bb);

        bool promote = to_bb & (RANK_1 | RANK_8);

        if (promote) {
            while (to_bb) {
                int to_sq = bit_scan(to_bb);
                push_promotions(move_arr, index, from_sq, to_sq, true);
                to_bb &= to_bb - 1;
            }
        } else {
            while (to_bb) {
                int to_sq = bit_scan(to_bb);
                move_flag flag = (to_sq == b->ep_square) ? EP_CAPTURE : CAPTURE;
                push_move(move_arr, index, from_sq, to_sq, flag);
                to_bb &= to_bb - 1;
            }
        }
        
        pawn_bb &= pawn_bb - 1;
    }
}

/* Helper function that populates the move array with knight moves. */
static void generate_knight_moves(move_t *move_arr, size_t *index, board *b) {
    side s = b->play_side;
    bitboard knight_bb = b->piece_bbs[KNIGHT][s];

    while (knight_bb) {
        int from_sq = bit_scan(knight_bb);
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

    int from_sq = bit_scan(king_bb);
    bitboard attack_bb = king_attacks[from_sq];
    push_attack_bb(move_arr, index, b, from_sq, attack_bb);

    // Castling
    if (is_in_check(b, s)) {
        return;
    }
    
    bool can_king_castle = b->castling & CASTLE_ARR_START >> (s << 1);
    bool can_queen_castle = b->castling & CASTLE_ARR_START >> (s << 1) >> 1;

    bitboard empty_bb = ~(b->occupied_bbs[WHITE] | b->occupied_bbs[BLACK]);
    bitboard king_castle_path = KING_CASTLE_PATH;
    bitboard queen_castle_path = QUEEN_CASTLE_PATH;

    if (s) {
        int shift_amt = (SIDE_LEN - 1) << VERT_SHIFT_POWER;
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

/* Helper function that generates a ray bitboard considering blockers, given a square and
 * ray direction.
 */
static bitboard generate_ray_bitboard(board *b, ray_dir dir, int square) {
    bitboard ray_bb = ray_attacks[square][dir];
    bitboard blockers_bb = ray_bb & (b->occupied_bbs[WHITE] | b->occupied_bbs[BLACK]);

    if (blockers_bb) {
        int block_sq = (rays[dir].negative) ? bit_scan_reverse(blockers_bb) : bit_scan(blockers_bb);
        ray_bb ^= ray_attacks[block_sq][dir];
    }

    return ray_bb;
}

/* Helper function that populates the move array with moves of any specified slider piece. */
static void generate_slider_moves(move_t *move_arr, size_t *index, board *b, piece_t p) {
    side s = b->play_side;
    bitboard piece_bb = b->piece_bbs[p][s];

    while (piece_bb) {
        int from_sq = bit_scan(piece_bb);
        bitboard attack_bb = 0;
        
        for (int i = 0; i < RAY_DIRS + 1; i++) {
            int ray_index = slider_ray_indices[p - BISHOP][i];
            if (ray_index == -1) break;

            attack_bb |= generate_ray_bitboard(b, (ray_dir)ray_index, from_sq);
        }

        push_attack_bb(move_arr, index, b, from_sq, attack_bb);
        piece_bb &= piece_bb - 1;
    }
}
