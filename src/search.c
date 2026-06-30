/* File: search.c
 * ---------------
 * For more information, see "search.h".
 */

#include <limits.h>
#include "search.h"

const int SEARCH_DEPTH = 4;

/* The `is_move_50` function returns whether or not the given position has reached
 * the 50-move rule, in which case the search should cut short and return a stalemate
 * evaluation (0).
 */
static inline bool is_50_move_rule(board *b) {
    return b->halfmove_clock >= 50;
}

/* The `no_moves_eval` function returns the evaluation, to be called when there are
 * no legal moves in a given position. Therefore, it returns a CHECKMATE_EVAL if
 * the player is in check or 0, meaning stalemate.
 */
static inline int no_moves_eval(board *b) {
    return (is_in_check(b, b->play_side)) ? -(CHECKMATE_EVAL - b->fullmove_counter) : 0;
}

/* The root and recursive calls must have slightly different implementations, and so
 * the `nega_max_helper` function represents the recursive calls of `nega_max`.
 */
static int nega_max_helper(board *b, int depth) {
    if (is_50_move_rule(b))
        return 0;
    
    if (depth == 0)
        return evaluate(b);
    
    move_t move_list[MAX_PLY];
    size_t n_moves;
    int max = INT_MIN;

    generate_moves(move_list, &n_moves, b);

    for (size_t i = 0; i < n_moves; i++) {
        make_move(b, move_list[i]);

        if (!is_in_check(b, b->play_side ^ 1)) {
            int score = -nega_max_helper(b, depth - 1);
            
            if (score > max) {
                max = score;
            }
        }

        unmake_move(b, move_list[i]);
    }

    if (max == INT_MIN)
        return no_moves_eval(b);

    return max;
}

move_t nega_max(board *b) {
    if (is_50_move_rule(b))
        return 0;
    
    move_t move_list[MAX_PLY];
    size_t n_moves;
    int max = INT_MIN;
    move_t best_move = NO_MOVE;
    
    generate_moves(move_list, &n_moves, b);

    for (size_t i = 0; i < n_moves; i++) {
        make_move(b, move_list[i]);

        if (!is_in_check(b, b->play_side ^ 1)) {
            int score = -nega_max_helper(b, SEARCH_DEPTH - 1);
            
            if (score > max) {
                best_move = move_list[i];
                max = score;
            }
        }

        unmake_move(b, move_list[i]);
    }

    return best_move;
}
