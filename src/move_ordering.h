/* File: move_ordering.h
 * ----------------------
 * This file implements move ordering algorithms for the chess engine,
 * important to use for alpha-beta pruning in the search.
 *
 * This project orders moves (in order of priority) by TT, MVV-LVA
 * captures, and the history heuristic.
 */

#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include <inttypes.h>
#include <limits.h>
#include "board.h"
#include "move.h"
#include "zobrist.h"
#include "transposition_table.h"
#include "history.h"

/* DEFINITIONS */

// Sentinel value used for marking moves that have already been searched.
#define SEARCHED_SCORE INT32_MIN
#define NON_CAPTURE_SCORE -MAX_HISTORY_SCORE
#define TT_SCORE INT32_MAX

/* Function: MVV_LVA
 * ------------------
 * The `MVV_LVA` function is a helper function that implements the
 * MVV-LVA (Most Valuable Victim - Least Valuable Aggressor) heuristic
 * for ordering capturing moves.
 */
static inline int32_t MVV_LVA(board *b, move_t move) {
    return get_piece_val(piece_on(b, get_to(move))) * 16 - get_piece_val(piece_on(b, get_from(move))); 
}

/* Function: score_moves
 * ----------------------
 * The `score_moves` function takes a list of move_t's and an empty
 * list of move_score's, and populates the scored_move list each with
 * the move_score associated with the move_t from the move_t list.
 */
static inline void score_moves(board *b, zobrist_board *game_history, move_t *move_list, int32_t *score_list, int n_moves) {
    // Get TT move
    move_t TT_move = NO_MOVE;
    zobrist_board zb = game_history[b->halfmove_clock];
    
    if (in_tt(zb)) {
        TT_move = get_tt_entry_move(*get_tt_entry(zb));
    }

    // Score moves
    for (int i = 0; i < n_moves; i++) {
        move_t cur_move = move_list[i];

        // TT
        if (cur_move == TT_move) {
            score_list[i] = TT_SCORE;
            continue;
        }

        // MVV-LVA
        if (is_capture(cur_move)) {
            score_list[i] = MVV_LVA(b, move_list[i]);
            continue;
        }

        // History
        score_list[i] = NON_CAPTURE_SCORE + get_history_score(b->play_side, cur_move);
    }
}

#endif
