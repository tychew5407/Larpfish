/* File: move_ordering.h
 * ----------------------
 * This file implements move ordering algorithms for the chess engine,
 * important to use for alpha-beta pruning in the search.
 *
 * Move ordering thus far is simply MVV-LVA.
 */

#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include <limits.h>
#include "board.h"
#include "move.h"

/* DEFINITIONS */

// Sentinel value used for marking moves that have already been searched.
#define SEARCHED_SCORE INT_MIN

// Sentinel value used for marking moves that are not captures.
#define NON_CAPTURE_SCORE -1000

/* Function: MVV_LVA
 * ------------------
 * The `MVV_LVA` function is a helper function that implements the
 * MVV-LVA (Most Valuable Victim - Least Valuable Aggressor) heuristic
 * for ordering capturing moves.
 */
static inline int MVV_LVA(board *b, move_t move) {
    if (!(get_flag(move) & CAPTURE_FLAG)) {
        return NON_CAPTURE_SCORE;
    }

    return get_piece_val(piece_on(b, get_to(move))) * 16 - get_piece_val(piece_on(b, get_from(move))); 
}

/* Function: score_moves
 * ----------------------
 * The `score_moves` function takes a list of move_t's and an empty
 * list of move_score's, and populates the scored_move list each with
 * the move_score associated with the move_t from the move_t list.
 */
static inline void score_moves(board *b, move_t *move_list, int *score_list, int n_moves) {
    for (int i = 0; i < n_moves; i++) {
        score_list[i] = MVV_LVA(b, move_list[i]);
    }
}

#endif
