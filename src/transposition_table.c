/* File: tranposition_table.c
 * ---------------------------
 * For more information, see "transposition_table.h".
 */

#include "transposition_table.h"
#include <stdlib.h>

/* The single instance of the transposition table that
 * this file uses.
 */
static transposition_table TT = {0};

/* HELPER FUNCTION PROTOTYPES */
static tt_val compact_tt_val(move_t best_move, int16_t score, int16_t eval, uint8_t depth, uint8_t age, tt_node_t type);

void init_tt(size_t size_MB) {
    assert(size_MB != 0);
    
    TT.capacity = (size_MB * 1000000) / sizeof(tt_entry);
    
    // Floor capacity to the nearest power of the 2
    size_t cap_MSB = 63 - __builtin_clzll(TT.capacity);
    TT.capacity = 1ULL << cap_MSB;
    
    TT.index_mask = TT.capacity - 1;

    TT.start = calloc(TT.capacity, sizeof(tt_entry));
    assert(TT.start);
}

void free_tt() {
    assert(TT.start);
    
    free(TT.start);
    TT.start = NULL;
}

bool tt_exists() {
    return TT.start != NULL;
}

tt_entry *create_tt_entry(zobrist_board key, move_t best_move, int16_t score, int16_t eval, uint8_t depth, uint8_t age, tt_node_t type) {
    assert(TT.start);
    
    tt_entry *elem = get_tt_entry(key);

    if (get_tt_entry_age(*elem) == age &&
        get_tt_entry_depth(*elem) > depth) {
        return NULL;
    }

    *elem = (tt_entry) {
        .key = key,
        .value = compact_tt_val(best_move, score, eval, depth, age, type)
    };

    return elem;
}

tt_entry *get_tt_entry(zobrist_board key) {
    assert(TT.start);

    size_t index = key & TT.index_mask;
    return &(TT.start[index]);
}

bool in_tt(zobrist_board key) {
    return get_tt_entry(key)->key == key;
}

static tt_val compact_tt_val(move_t best_move, int16_t score, int16_t eval, uint8_t depth, uint8_t age, tt_node_t type) {
    tt_val ret =
        ((uint64_t)best_move << 48) |
        ((uint64_t)(uint16_t)score << 32) |
        ((uint64_t)(uint16_t)eval << 16) |
        ((uint64_t)(depth & 0x7F) << 9) |
        ((age & 0x7F) << 2) |
        type;

    return ret;
}
