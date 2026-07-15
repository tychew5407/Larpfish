/* File: search.h
 * ---------------
 * This file implements the search algorithm for the chess engine.
 * This project currently using an alpha-beta pruned Negamax algorithm
 * with iterative deepening.
 */

#ifndef SEARCH_H
#define SEARCH_H

#include <stdatomic.h>
#include "definitions.h"
#include "board.h"
#include "move.h"
#include "move_make.h"
#include "movegen.h"
#include "evaluation.h"

/* DEFINITIONS */
#define CHECKMATE_EVAL 1000
#define ABORTED_EVAL 9999 // sentinel value when search is aborted

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
 *
 * The function also supports `n_searched` which, when not NULL, is populated with the
 * number of nodes considered by the search function.
 */
int search(board *b, zobrist_board *game_history, move_t *best_move, uint64_t *n_searched, int depth);

/* Function: find_first_legal
 * ---------------------------
 * The `find_first_legal` function returns the eval of the first legal move that can be
 * found, used as a last-ditch effort when the search aborts before any moves were found.
 */
move_t find_first_legal(board *b, zobrist_board *game_history);

#endif
