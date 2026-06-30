/* File: search.h
 * ---------------
 * This file implements the search algorithm for the chess engine.
 * This project currently using a Negamax algorithm.
 */

#ifndef SEARCH_H
#define SEARCH_H

#include "definitions.h"
#include "board.h"
#include "move.h"
#include "move_make.h"
#include "movegen.h"
#include "evaluation.h"

/* DEFINITIONS */
#define CHECKMATE_EVAL 1000

/* The `nega_max` function takes a board pointer, move pointer, and specified depth,
 * and outputs the score of the best move according to the evaluation function. It
 * also sets `move` to the best move.
 */
move_t nega_max(board *b);

#endif
