/* File: attack_tables.c
 * ----------------------
 * For further information, see "attack_tables.h".
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include "attack_tables.h"
#include "definitions.h"
#include "random.h"

/* DEFINITIONS */

/* These definitions of ray_dir and ray are used to compute
 * sliding attacks via the classical ray method.
 */
typedef enum {
    NORTH,
    NORTHEAST,
    EAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    WEST,
    NORTHWEST
} ray_dir;

typedef struct {
    ray_dir dir;
    size_t shift_amount; /* The # of squares, or bits, to shift to go one square
                          * in the direction of `dir`.
                          */
    bool negative;   /* If we are traversing squares in the negative direction,
                      * we have to bitshift leftwards since bitshifting right by
                      * a negative amount is not supported.
                      */
    bitboard bounds; /* The bitboard ANDED against the square obtained by traveling
                      * in the direction of `dir`. Primarily used to prevent wrap-arounds.
                      */
} ray;

/* GLOBAL VARIABLES */
bitboard pawn_attacks[NUM_SIDES][NUM_SQUARES] = {0};
bitboard knight_attacks[NUM_SQUARES] = {0};
bitboard king_attacks[NUM_SQUARES] = {0};
magic rook_magics[NUM_SQUARES] = {0};
magic bishop_magics[NUM_SQUARES] = {0};

// Both rays and bounds are enumerated via ray_dirs.
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

/* The bounds for occupancy masks in computing magic
 * numbers are stricter than the standard ray approach.
 * We only care about the positions in which a piece occupying
 * that square will affect the sliding piece's movement.
 */
static const bitboard occ_bounds[RAY_DIRS] = {
    ~RANK_8,            // NORTH
    ~RANK_8 & ~FILE_H,  // NORTHEAST
    ~FILE_H,            // EAST
    ~RANK_1 & ~FILE_H,  // SOUTHEAST
    ~RANK_1,            // SOUTH
    ~RANK_1 & ~FILE_A,  // SOUTHWEST
    ~FILE_A,            // WEST
    ~RANK_8 & ~FILE_A   // NORTHWEST
};

static const ray_dir rook_rays[NUM_ROOK_RAYS] = {NORTH, EAST, SOUTH, WEST};
static const ray_dir bishop_rays[NUM_BISHOP_RAYS] = {NORTHEAST, SOUTHEAST, SOUTHWEST, NORTHWEST};

static bitboard ray_attacks[NUM_SQUARES][RAY_DIRS] = {0};

/* This version of the slider attacks table is computed via
 * classical rays. This is used purely for verifying correctness
 * during magic number computation.
 */
static bitboard *slider_attacks_ray = NULL;

/* The slider attacks table computed via magic numbers, accessed
 * by the rook_magics and bishop_magics tables.
 */
static bitboard *slider_attacks_magic = NULL;

static const bool FIND_NEW_MAGICS = false;

// Precomputed magic numbers
static const uint64_t ROOK_MAGIC_NUMS[NUM_SQUARES] = {
    0x1880008440002010, 0x40400010002000, 0x200114080202a00, 0x8080080010008094,
    0x200080c20300200, 0x100040008010002, 0x8200080200108104, 0x8180002050800500,
    0x10802080004004, 0xb002404000201000, 0x41002001019040, 0x509002300100008,
    0x101000408010010, 0x8182800400420080, 0x104000448211002, 0x100800100006080,
    0x84808000400120, 0x840414000201000, 0x10008020008010, 0xe80221001001000c,
    0x2041010011044800, 0x105818004000600, 0x10600c0002300108, 0x4160e000283004c,
    0xa080010100208040, 0x818400900210080, 0x2220120200208041, 0x8040520200204008,
    0x400080100250010, 0xc008080020004, 0x205000100040200, 0x8320420008a104,
    0x44400120c0800280, 0x8000200042401004, 0x4800811004802004, 0x8008208801004,
    0x26800800804400, 0x402000506001018, 0x5011014002288, 0x2002204882001401,
    0x80a0400880028021, 0x2001400020008080, 0x30a002070820040, 0x2080080010008080,
    0x1000480005010010, 0x1102005410260008, 0xc408040200010100, 0x80414c008a0021,
    0x26304004800080, 0x3220003040008080, 0x2028401020010100, 0x10040108004040,
    0x2002c20281200, 0x101000400085300, 0x852000841440200, 0x8008010044128a00,
    0x10604081020a, 0x248140025101, 0x44008c4100502001, 0x50100200c089001,
    0x3021004800020411, 0x2001000208040001, 0x10820130080084, 0x2022648100402c02
};

static const uint64_t BISHOP_MAGIC_NUMS[NUM_SQUARES] = {
    0x2622401020a0200, 0x208a101c29104110, 0x1009261a200010, 0x244040180000080,
    0x14042010000000, 0x181100610000110, 0x4002808808400000, 0x4022002608022828,
    0x410041410020200, 0x2082302431204600, 0x1010240401820000, 0xa1082244404040,
    0x10140420082002, 0x80010120900000, 0x4010808051400, 0x4054822d84100808,
    0x86c86048100108, 0x20082206040500, 0x1204000808002098, 0x8010800812024000,
    0x8044040484a00004, 0x8407005080600600, 0xa00040d040380, 0xc20228101080262,
    0x1820600508082900, 0x1441843008104400, 0x61e29000080020a2, 0x443802008020020,
    0x6040002018208, 0x10100100028080c0, 0x4008001080100, 0x420800808440,
    0x2405042030102001, 0x1204100220046482, 0x4860a801501480, 0x901110800440041,
    0x804010202040084, 0x20004480010082, 0x82040c26c1008809, 0x812908a010041,
    0x8200840420804000, 0x8001080104041000, 0x408110801000804, 0x882011020800,
    0x4080080104054040, 0x422040812000021, 0x12020a02018420, 0x10020a0042008118,
    0x8903040e66400200, 0x20100ca100200, 0x50a41100000, 0x8011150384110290,
    0x2002002440401, 0x101480608020002, 0x10830801140002, 0xc808700c4082a100,
    0x121088042a2000, 0x8204060121821084, 0x2000040040441080, 0x2008040201840420,
    0x2000000108210104, 0x4ed002060424080, 0x200c00401021200, 0xb6020060202a020
};

/* FUNCTION PROTOTYPES */
static void init_pawn_attacks();
static void init_knight_attacks();
static void init_king_attacks();
static void init_ray_attacks();
static bitboard generate_ray_attack(const int sq, const ray_dir dir, const bitboard blockers_bb);
static void init_slider_attacks_ray();
static void init_magics();
static bitboard generate_ray_attack(const int sq, const ray_dir dir, const bitboard blockers_bb);
static bitboard generate_slider_attack(const int sq, const bitboard blockers_bb, const ray_dir rays[], const size_t n_rays);
static void fill_slider_ray_table(bitboard slider_table[], size_t *table_index, const ray_dir rays[], const size_t n_rays);
static uint64_t find_magic_num(magic *m, size_t *ray_index, xorshift64_state *rng);
static bool test_magic_num(magic *m, uint64_t magic_num, size_t *ray_index);
static void fill_magic_table(const ray_dir rays[], const size_t n_rays, const uint64_t magic_nums[], magic magics_table[], size_t *magic_index, size_t *ray_index, xorshift64_state *rng);

/* ------------------------------- */

void init_attack_tables() {
    init_pawn_attacks();
    init_knight_attacks();
    init_king_attacks();
    init_magics();
}

void free_sliding_attacks() {
    if (slider_attacks_ray) free(slider_attacks_ray);
    if (slider_attacks_magic) free(slider_attacks_magic);
}

// Helper function to compute the `pawn_attacks` table.
static void init_pawn_attacks() {
    for (int sq = 0; sq < NUM_SQUARES; sq++) {
        bitboard sq_bb = 1ULL << sq;
        
        pawn_attacks[WHITE][sq] = ((sq_bb & ~FILE_A) << VERT_SHIFT >> 1) | ((sq_bb & ~FILE_H) << VERT_SHIFT << 1);
        pawn_attacks[BLACK][sq] = ((sq_bb & ~FILE_A) >> VERT_SHIFT >> 1) | ((sq_bb & ~FILE_H) >> VERT_SHIFT << 1);
    }
}

// Helper function to compute the 'knight_attacks' table. 
static void init_knight_attacks() {
    for (int sq = 0; sq < NUM_SQUARES; sq++) {
        bitboard sq_bb = 1ULL << sq;

        bitboard dir_bbs[KNIGHT_DIRS] = {
            (sq_bb & ~FILE_H) << (VERT_SHIFT << 1) << 1,    // NNE
            (sq_bb & ~FILE_A) << (VERT_SHIFT << 1) >> 1,    // NNW
            (sq_bb & ~FILE_G & ~FILE_H) << VERT_SHIFT << 2, // NEE
            (sq_bb & ~FILE_A & ~FILE_B) << VERT_SHIFT >> 2, // NWW
            (sq_bb & ~FILE_H) >> (VERT_SHIFT << 1) << 1,    // SSE
            (sq_bb & ~FILE_A) >> (VERT_SHIFT << 1) >> 1,    // SSW
            (sq_bb & ~FILE_G & ~FILE_H) >> VERT_SHIFT << 2, // SEE
            (sq_bb & ~FILE_A & ~FILE_B) >> VERT_SHIFT >> 2  // SWW
        };

        for (int i = 0; i < KNIGHT_DIRS; i++) {
            knight_attacks[sq] |= dir_bbs[i];
        }
    }
}

// Helper function to compute the `king_attacks` table.
static void init_king_attacks() {
    for (int sq = 0; sq < NUM_SQUARES; sq++) {
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

// Helper function to compute the internal `ray_attacks` table.
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

            while (ray_bb & cur.bounds) {
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

// Helper function to initialize and compute the `slider_attacks_ray` table.
static void init_slider_attacks_ray() {
    size_t cur_table_index = 0;
    slider_attacks_ray = malloc(sizeof(bitboard) * NUM_SLIDING_ATTACKS);
    assert(slider_attacks_ray);

    fill_slider_ray_table(slider_attacks_ray, &cur_table_index, rook_rays, NUM_ROOK_RAYS);
    fill_slider_ray_table(slider_attacks_ray, &cur_table_index, bishop_rays, NUM_BISHOP_RAYS);
}

// Helper function to compute the `rook_magics` and `bishop_magics` table.
static void init_magics() {
    xorshift64_state rng;
    xorshift64_init(&rng, (uint64_t)time(NULL));
    
    init_ray_attacks();
    init_slider_attacks_ray();
    size_t ray_index = 0;
    
    slider_attacks_magic = calloc(NUM_SLIDING_ATTACKS, sizeof(bitboard));
    assert(slider_attacks_magic);
    size_t magic_index = 0;
    
    fill_magic_table(rook_rays, NUM_ROOK_RAYS, ROOK_MAGIC_NUMS, rook_magics, &magic_index, &ray_index, &rng);
    fill_magic_table(bishop_rays, NUM_BISHOP_RAYS, BISHOP_MAGIC_NUMS, bishop_magics, &magic_index, &ray_index, &rng);
    
    free(slider_attacks_ray);
    slider_attacks_ray = NULL;
}

/* Helper function that generates a ray attack bitboard with
 * consideration of a given `blockers_bb`.
 */
static bitboard generate_ray_attack(const int sq, const ray_dir dir, const bitboard blockers_bb) {
    bitboard ray_bb = ray_attacks[sq][dir];
    bitboard ray_blockers_bb = ray_bb & blockers_bb;
    
    if (ray_blockers_bb) {
        int block_sq = (rays[dir].negative) ? bit_scan_reverse(ray_blockers_bb) : bit_scan_forward(ray_blockers_bb);
        ray_bb ^= ray_attacks[block_sq][dir];
    }

    return ray_bb;
}

/* Helper function that returns an "occupancy mask" of `ray_bb`.
 * In which only squares that affect a sliding piece's movement
 * in the ray direction are 1s in the resulting bitboard.
 */
static inline bitboard get_occ_mask(bitboard ray_bb, ray_dir dir) {
    return ray_bb & occ_bounds[dir];
}

/* Helper function that uses the classical ray method to compute a
 * sliding piece's attack bitboard.
 */
static bitboard generate_slider_attack(const int sq, const bitboard blockers_bb, const ray_dir rays[], const size_t n_rays) {
    bitboard slider_bb = 0;
    
    for (size_t i = 0; i < n_rays; i++) {
        slider_bb |= generate_ray_attack(sq, rays[i], blockers_bb);
    }

    return slider_bb;
}

/* Helper function that uses the classical ray method to compute a portion
 * of `slider_table` for one slider piece.
 */
static void fill_slider_ray_table(bitboard slider_table[], size_t *table_index, const ray_dir rays[], const size_t n_rays) {
    assert(slider_table);
    
    for (int sq = 0; sq < NUM_SQUARES; sq++) {
        bitboard occ_bb = 0;
        for (size_t i = 0; i < n_rays; i++) {
            ray_dir cur_dir = rays[i];
            occ_bb |= get_occ_mask(ray_attacks[sq][cur_dir], cur_dir);
        }

        bitboard occ_subset = occ_bb;
        while (true) {
            slider_table[*table_index] = generate_slider_attack(sq, occ_subset, rays, n_rays);
            (*table_index) ++;

            if (occ_subset) {
                occ_subset = (occ_subset - 1) & occ_bb;
            } else {
                break;
            }
        }
    }
}

// Helper function to compute a magic number for a given magic struct `m`.
static uint64_t find_magic_num(magic *m, size_t *ray_index, xorshift64_state *rng) {
    assert(slider_attacks_ray);
    assert(m);
    assert(m->attack_table);
    
    uint64_t magic_num;
        
    while (true) {            
        magic_num = xorshift64(rng) & xorshift64(rng) & xorshift64(rng);
        if (test_magic_num(m, magic_num, ray_index)) break;
    }

    return magic_num;
}

/* Helper function to get the corresponding magic number from a precomputed `magic_nums` table,
 * filling in the `slider_attacks_ray` table as necessary.
 */
static bool test_magic_num(magic *m, uint64_t magic_num, size_t *ray_index) {
    bitboard occ_subset = m->occ_mask;
    bool magic_valid = true;

    size_t starting_ray_index = *ray_index;
    size_t attack_table_size = 1ULL << (64 - m->shift);

    // Zero out table portion
    for (size_t i = 0; i < attack_table_size; i++) {
        m->attack_table[i] = 0;
    }
        
    while (true) {
        bitboard attack_bb = slider_attacks_ray[*ray_index];
        uint64_t cur_index = (occ_subset * magic_num) >> m->shift;

        assert(cur_index < attack_table_size);
        
        bitboard *magic_bb = &(m->attack_table[cur_index]);

        if (*magic_bb == 0 || *magic_bb == attack_bb) {
            *magic_bb = attack_bb;
        } else {
            magic_valid = false;
            *ray_index = starting_ray_index;
            break;
        }
        
        (*ray_index)++;
        
        if (occ_subset) {
            occ_subset = (occ_subset - 1) & m->occ_mask;
        } else {
            break;
        }
    }

    return magic_valid;
}

// Helper function to fill a portion of a given `magics_table` for one slider piece.
static void fill_magic_table(const ray_dir rays[], const size_t n_rays, const uint64_t magic_nums[], magic magics_table[], size_t *magic_index, size_t *ray_index, xorshift64_state *rng) {
    assert(slider_attacks_ray);
    assert(slider_attacks_magic);

    for (int sq = 0; sq < NUM_SQUARES; sq++) {
        bitboard occ_bb = 0;
        for (size_t i = 0; i < n_rays; i++) {
            ray_dir cur_dir = rays[i];
            occ_bb |= get_occ_mask(ray_attacks[sq][cur_dir], cur_dir);
        }

        size_t num_blockers = __builtin_popcountll(occ_bb);
       
        magics_table[sq].occ_mask = occ_bb;
        magics_table[sq].attack_table = &(slider_attacks_magic[*magic_index]);
        magics_table[sq].shift = 64 - num_blockers;
        
        if (FIND_NEW_MAGICS) {
            magics_table[sq].magic_num = find_magic_num(&magics_table[sq], ray_index, rng);
        } else {
            bool magic_valid = test_magic_num(&magics_table[sq], magic_nums[sq], ray_index);
            assert(magic_valid);
            magics_table[sq].magic_num = magic_nums[sq];
        }
        
        
        *magic_index += 1ULL << num_blockers;
    }
}
