/* File: search.c
 * ---------------
 * For more information, see "search.h".
 */

#include <limits.h>
#include <stdbool.h>
#include "search.h"

atomic_bool search_running = false;

/* The `is_move_50` function returns whether or not the given position has reached
 * the 50-move rule, in which case the search should cut short and return a stalemate
 * evaluation (0).
 */
static inline bool is_50_move_rule(board *b) {
    return b->halfmove_clock >= MAX_HALFMOVES;
}

/* The `no_moves_eval` function returns the evaluation, to be called when there are
 * no legal moves in a given position. Therefore, it returns a CHECKMATE_EVAL if
 * the player is in check or 0, meaning stalemate.
 */
static inline int no_moves_eval(board *b) {
    return (is_in_check(b, b->play_side)) ? -(CHECKMATE_EVAL - b->fullmove_counter) : 0;
}

/* The `is_repeat` function returns whether the last element of `game_history`
 * (the current position) is a duplicate of any other element in the array. Returning
 * true would mean that the current position is a repeated position from earlier in
 * the game.
 */
static inline bool is_repeat(board *b, zobrist_board *game_history) {
    zobrist_board cur_pos = game_history[b->halfmove_clock];
    for (int i = b->halfmove_clock - 2; i >= 0; i -= 2) {
        if (game_history[i] == cur_pos) {
            return true;
        }
    }
    return false;
}

static int alpha_beta(board *b, zobrist_board *game_history, move_t *best_move, int alpha, int beta, int depth) {
    if (!atomic_load(&search_running)) {
        return ABORTED_EVAL;
    }
    
    if (is_repeat(b, game_history)) {
        return 0;
    }
    
    if (depth == 0) {
        return (is_50_move_rule(b)) ? 0 : evaluate(b);
    }

    move_t move_list[MAX_MOVES];
    size_t n_moves;
    int best_score = INT_MIN;

    generate_moves(move_list, &n_moves, b);

    for (size_t i = 0; i < n_moves; i++) {
        make_move(b, game_history, move_list[i]);

        if (!is_in_check(b, b->play_side ^ 1)) {
            int score = -alpha_beta(b, game_history, NULL, -beta, -alpha, depth - 1);

            if (score == ABORTED_EVAL || score == -ABORTED_EVAL) {
                unmake_move(b, game_history, move_list[i]);
                break;
            }
            
            if (score > best_score) {
                best_score = score;
                if (best_move) {
                    *best_move = move_list[i];
                }
                if (score > alpha) {
                    alpha = score;
                }
            }

            if (score >= beta) {
                unmake_move(b, game_history, move_list[i]);
                return best_score;
            }
        }

        unmake_move(b, game_history, move_list[i]);
    }

    if (best_score == INT_MIN) {
        return no_moves_eval(b);
    }
       

    if (is_50_move_rule(b)) {
        return 0;
    }

    return best_score;
}

int search(board *b, zobrist_board *game_history, move_t *best_move, int depth) {
    return alpha_beta(b, game_history, best_move, -INT_MAX, INT_MAX, depth);
}
