/* File: search.h
 * ---------------
 * This file implements the search algorithm for the chess engine.
 * This project currently using a Negamax algorithm.
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

/* The `nega_max` function takes a board pointer, move pointer, and specified depth,
 * and outputs the score of the best move according to the evaluation function. It
 * also sets `best_move` to the best move.
 *
 * `best_move` should ideally be initialized to NO_MOVE, so that there is indication
 * of whether it was modified (aka if search yielded results) or not.
 */
int nega_max(board *b, zobrist_board *game_history, move_t *best_move, int depth);

#endif
