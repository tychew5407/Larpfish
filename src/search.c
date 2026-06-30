/* File: search.c
 * ---------------
 * For more information, see "search.h".
 */

#include <limits.h>
#include "search.h"

const int SEARCH_DEPTH = 4;

// Where the recrusive algorithm is implemented.
static int nega_max_helper(board *b, int depth) {
    move_t move_list[MAX_PLY];
    size_t n_moves;

    generate_moves(move_list, &n_moves, b);
    
    if (depth == 0) {
        return evaluate(b);
    }

    int max = INT_MIN;

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

    if (max == INT_MIN) {
        if (is_in_check(b, b->play_side)) {
            return -CHECKMATE_EVAL;
        } else {
            return 0;
        }
    }

    return max;
}

move_t nega_max(board *b) {
    move_t move_list[MAX_PLY];
    size_t n_moves;

    generate_moves(move_list, &n_moves, b);

    int max = INT_MIN;
    move_t best_move = NO_MOVE;

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
