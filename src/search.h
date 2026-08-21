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
#include "move_make.h"
#include "movegen.h"
#include "evaluation.h"

/* DEFINITIONS */
#define CHECKMATE_EVAL (INT16_MAX - 1)
#define ABORTED_EVAL INT16_MAX // sentinel value when search is aborted
#define NO_EVAL INT16_MIN /* sentinel value to indicate that a static evaluation was not recorded
                             on the eval stack, aka the position is in check. */
#define NO_TT_SCORE INT16_MAX  // sentinel value when TT lookup score cannot be used or is not foun
#define RFP_MARGIN 175           // margin constant for reverse futility pruning
#define IMPROVING_RFP_MARGIN 135 // margin constant for RFP when position is improving
#define RFP_DEPTH_BOUND 4      // depth bound constant for reverse futility pruning
#define ASPIRATION_WINDOW_DELTA_DEFAULT 50

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

/* Function: search
 * -----------------
 * The `search` function outputs the score of the best move of the current board
 * position according to the evaluation function, setting `best_move` to the best move.
 *
 * `best_move` should ideally be initialized to NO_MOVE, so that there is indication
 * of whether it was modified (aka if search yielded results) or not.
 */
move_t search(search_context *context, uint8_t max_depth);

#endif
