/* File: search.h
 * ---------------
 * This file implements the search algorithm for the chess engine.
 * This project currently using an alpha-beta pruned Negamax algorithm
 * with iterative deepening.
 */

#ifndef SEARCH_H
#define SEARCH_H

#include <inttypes.h>
#include <stdatomic.h>
#include "definitions.h"
#include "board.h"
#include "move.h"
#include "zobrist.h"

typedef struct {
    board *game_board;
    zobrist_board *game_history;
    uint64_t *n_searched;
    uint8_t age;
} search_context;

typedef struct {
    int16_t alpha;
    int16_t beta;
} search_window;

/* This global atomic_bool is used for UCI-support, where there are instances in which
 * the search may need to exit prematurely.
 */
extern atomic_bool search_running;

/* Function: init_search_tables
 * ----------------------------
 * The `init_search_tables` function computes and populates the internal lookup
 * tables used by the search function. Should be called once during engine startup.
 */
void init_search_tables();

/* Function: search
 * -----------------
 * The `search` function outputs the score of the best move of the current board
 * position according to the evaluation function, setting `best_move` to the best move.
 *
 * `best_move` should ideally be initialized to NO_MOVE, so that there is indication
 * of whether it was modified (aka if search yielded results) or not.
 */
move_t search(search_context *context, uint8_t max_depth);

/* Function: q_search
 * -------------------
 * The `q_search` function outputs the score of the current board position according
 * to the quiescence search. No move is returned. This is used for Texel tuning purposes.
 */
int16_t q_search(search_context *context);

#endif
