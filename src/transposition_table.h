/* File: transposition_table.h
 * -----------------------------
 * This file implements the engine's transposition table (TT), used for
 * storing search information about positions already encountered by the
 * search algorithm.
 */

#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <stdint.h>
#include "zobrist.h"
#include "move.h"

/* DEFINITIONS */
#define TT_MOVE_MASK (0xFFFFULL << 48)
#define TT_SCORE_MASK (0xFFFFULL << 32)
#define TT_EVAL_MASK (0xFFFFULL << 16)
#define TT_DEPTH_MASK (0x7F << 9)
#define TT_AGE_MASK (0x7F << 2)
#define TT_TYPE_MASK 0x3

/* The transposition node type (tt_node_t) describes the type of node
 * found when writing a TT entry:
 *   - PV_NODE: the reported score is exact, found in a principal variation.
 *   - CUT_NODE: the reported score is a lower bound, found on nodes that
 *               experienced a beta cutoff.
 *   - ALL_NODE: the reported score is an upper bound, found on nodes that
 *               experienced an alpha cutoff.
 */
typedef enum {
    PV_NODE,
    CUT_NODE,
    ALL_NODE
} tt_node_t;

/* `tt_val` is a 64-bit compaction of aspects of a TT node
 * with the following fields:
 *   - best_move: The best move found during the search, 16 bits
 *   - score: The evaluated score of the node from the tree search, 16 bits
 *   - eval: The static evaluation of the node, 16 bits
 *   - depth: Depth of node during search, 7 bits
 *   - age: Halfmove_clock of root search, 7 bits
 *   - type: See tt_node_t, 2 bits
 */
typedef uint64_t tt_val; 

/* Each entry in the TT are 16 bytes big and store the zobrist_board to confirm
 * the position and its values.
 */
typedef struct {
    zobrist_board key;
    tt_val value;
} tt_entry;

/* The `transposition_table` struct is a representation of the allocated
 * transposition table:
 *   - start: a pointer to the start of the transposition table on the heap.
 *   - capacity: the # of elements that the transposition table can hold.
 *   - index_mask: used to mask out a specified # of lower bits to use as a key
 *                 for a given Zobrist hash, calculated by capacity log 2.
 */
typedef struct {
    tt_entry *start;
    uint64_t capacity;
    uint64_t index_mask;
} transposition_table;

/* FUNCTION PROTOYPES */

/* Function: init_tt
 * ------------------
 * The `init_tt` function initializes the transposition table by
 * dynamically allocating the specified `size_MB` megabytes and zeroing
 * out the table. Should be called upon starting every new game.
 *
 * The actual capacity of the transposition table (# of elements) gets
 * rounded down to the nearest power of 2 for indexing purposes.
 */
void init_tt(size_t size_MB);

/* Function: free_tt
 * ------------------
 * The `free_tt` function frees the transposition table from the
 * heap. Should be called upon game ends or quits.
 */
void free_tt();

/* Function: tt_exists
 * --------------------
 * The `tt_exists` function returns whether the transposition table
 * exists (currently allocated on the heap).
 */
bool tt_exists();

/* Function: create_tt_entry
 * -------------------------
 * The `create_tt_entry` function takes the specified fields required and returns a
 * pointer to the tt_entry struct on the transposition table.
 *
 * If no entry was created due to existing, more important information already being
 * present on the table's index, the function instead returns NULL. This engine
 * considers age and depth for its TT replacement scheme.
 */
tt_entry *create_tt_entry(zobrist_board key, move_t best_move, int16_t score, int16_t eval, uint8_t depth, uint8_t age, tt_node_t type);

/* Function: get_tt_entry
 * -----------------------
 * The `get_tt_entry` function returns a pointer to the tt_entry from the tranposition
 * table, taking in a zobrist-hashed board as a key.
 */
tt_entry *get_tt_entry(zobrist_board key);

/* Function: in_tt
 * ----------------
 * The `in_tt` function returns whether or not the given Zobrist key already has
 * an entry in the transposition table.
 */
bool in_tt(zobrist_board key);

/* --------------------------------------------------------------------------------------- */

/* Getter functions below for accessing different fields from the value
 * of transposition table entries.
 */
static inline move_t get_tt_entry_move(tt_entry entry) {
    return (move_t)((entry.value & TT_MOVE_MASK) >> 48);
}

static inline int16_t get_tt_entry_score(tt_entry entry) {
    return (int16_t)((entry.value & TT_SCORE_MASK) >> 32);
}

static inline int16_t get_tt_entry_eval(tt_entry entry) {
    return (int16_t)((entry.value & TT_EVAL_MASK) >> 16);
}

static inline uint8_t get_tt_entry_depth(tt_entry entry) {
    return (uint8_t)((entry.value & TT_DEPTH_MASK) >> 9);
}

static inline uint8_t get_tt_entry_age(tt_entry entry) {
    return (uint8_t)((entry.value & TT_AGE_MASK) >> 2);
}

static inline tt_node_t get_tt_entry_type(tt_entry entry) {
    return (tt_node_t)(entry.value & TT_TYPE_MASK);
}

#endif
