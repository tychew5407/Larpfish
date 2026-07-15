/* File: search.c
 * ---------------
 * For more information, see "search.h".
 */

#include <limits.h>
#include <stdbool.h>
#include "search.h"
#include "move_ordering.h"

atomic_bool search_running = false;

/* FUNCTION PROTOTYPES */
static inline bool is_50_move_rule(board *b);
static inline int no_moves_eval(board *b);
static inline bool is_repeat(board *b, zobrist_board *game_history);
static inline move_t get_next_move(move_t *move_list, int *score_list, size_t n_moves);
static int alpha_beta(board *b, zobrist_board *game_history, move_t *best_move, uint64_t *n_searched, int alpha, int beta, int depth);
static int quiesce(board *b, zobrist_board *game_history, uint64_t *n_searched, int alpha, int beta);

int search(board *b, zobrist_board *game_history, move_t *best_move, uint64_t *n_searched, int depth) {
    if (n_searched) {
        *n_searched = 0;
    }
    
    return alpha_beta(b, game_history, best_move, n_searched, -INT_MAX, INT_MAX, depth);
}

move_t find_first_legal(board *b, zobrist_board *game_history) {
    move_t move_list[MAX_MOVES];
    size_t n_moves;
        
    generate_moves(move_list, &n_moves, b);

    for (size_t i = 0; i < n_moves; i++) {
        make_move(b, game_history, move_list[i]);

        if (!is_in_check(b, b->play_side ^ 1)) {
            unmake_move(b, game_history, move_list[i]);
            return move_list[i];
        }

        unmake_move(b, game_history, move_list[i]);
    }

    return NO_MOVE;
}

/* Function: alpha_beta
 * ---------------------
 * The `alpha_beta` function is the implementation of the actual
 * alpha_beta Negamax algorithm, which is called by search().
 */
static int alpha_beta(board *b, zobrist_board *game_history, move_t *best_move, uint64_t *n_searched, int alpha, int beta, int depth) {
    if (!atomic_load(&search_running)) {
        return ABORTED_EVAL;
    }
    
    if (is_repeat(b, game_history)) {
        return 0;
    }
    
    if (depth == 0) {
        return (is_50_move_rule(b)) ? 0 : quiesce(b, game_history, n_searched, alpha, beta);
    }

    move_t move_list[MAX_MOVES];
    int score_list[MAX_MOVES];
    size_t n_moves;
    int best_score = INT_MIN;

    generate_moves(move_list, &n_moves, b);
    score_moves(b, move_list, score_list, n_moves);

    for (size_t i = 0; i < n_moves; i++) {
        move_t cur_move = get_next_move(move_list, score_list, n_moves);
        assert(cur_move != NO_MOVE);
        
        if (n_searched) {
            (*n_searched)++;
        }
        
        make_move(b, game_history, cur_move);

        if (!is_in_check(b, b->play_side ^ 1)) {
            int score = -alpha_beta(b, game_history, NULL, n_searched, -beta, -alpha, depth - 1);

            if (score == ABORTED_EVAL || score == -ABORTED_EVAL) {
                unmake_move(b, game_history, cur_move);
                break;
            }
            
            if (score > best_score) {
                best_score = score;
                if (best_move) {
                    *best_move = cur_move;
                }
                if (score > alpha) {
                    alpha = score;
                }
            }

            if (score >= beta) {
                unmake_move(b, game_history, cur_move);
                return best_score;
            }
        }

        unmake_move(b, game_history, cur_move);
    }

    if (best_score == INT_MIN) {
        return no_moves_eval(b);
    }
       

    if (is_50_move_rule(b)) {
        return 0;
    }

    return best_score;
}

/* Function: quiesce
 * ------------------
 * The `quiesce` function performs a quiesce search, performed at the leaf-nodes
 * of the alpha-beta Negamax search to avoid the horizon effect.
 */
static int quiesce(board *b, zobrist_board *game_history, uint64_t *n_searched, int alpha, int beta) {
    int static_eval = evaluate(b);

    if (!atomic_load(&search_running)) {
        return ABORTED_EVAL;
    }
    
    move_t move_list[MAX_MOVES];
    int score_list[MAX_MOVES];
    size_t n_moves;

    // Stand pat
    int best_score = static_eval;
    
    if (best_score >= beta) {
        return best_score;
    }

    if (best_score > alpha) {
        alpha = best_score;
    }

    generate_moves(move_list, &n_moves, b);
    score_moves(b, move_list, score_list, n_moves);

    for (size_t i = 0; i < n_moves; i++) {
        move_t cur_move = get_next_move(move_list, score_list, n_moves);
        assert(cur_move != NO_MOVE);

        if (!(get_flag(cur_move) & CAPTURE_FLAG)) break;
        
        if (n_searched) {
            (*n_searched)++;
        }
        
        make_move(b, game_history, cur_move);

        if (!is_in_check(b, b->play_side ^ 1)) {
            int score = -quiesce(b, game_history, n_searched, -beta, -alpha);

            if (score == ABORTED_EVAL || score == -ABORTED_EVAL) {
                unmake_move(b, game_history, cur_move);
                break;
            }
            
            if (score > best_score) {
                best_score = score;
                
                if (score > alpha) {
                    alpha = score;
                }
            }

            if (score >= beta) {
                unmake_move(b, game_history, cur_move);
                return best_score;
            }
        }

        unmake_move(b, game_history, cur_move);
    }

    return best_score;
}

/* Function: is_50_move_rule
 * --------------------------
 * The `is_50_move_rule` function returns whether or not the given position has reached
 * the 50-move rule, in which case the search should cut short and return a stalemate
 * evaluation (0).
 */
static inline bool is_50_move_rule(board *b) {
    return b->halfmove_clock >= MAX_HALFMOVES;
}

/* Function: no_moves_eval
 * -----------------------
 * The `no_moves_eval` function returns the evaluation, to be called when there are
 * no legal moves in a given position. Therefore, it returns a CHECKMATE_EVAL if
 * the player is in check or 0, meaning stalemate.
 */
static inline int no_moves_eval(board *b) {
    return (is_in_check(b, b->play_side)) ? -(CHECKMATE_EVAL - b->fullmove_counter) : 0;
}

/* Function: is_repeat
 * --------------------
 * The `is_repeat` function returns whether the last element of `game_history`
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

/* Function: get_next_move
 * ------------------------
 * The `get_next_move` function returns the next unused move that has the maximal
 * move ordering score according to `score_list`. Modifies `score_list` to mark the
 * next move as already being searched.
 */
static inline move_t get_next_move(move_t *move_list, int *score_list, size_t n_moves) {
    int max_index = -1;
    int max_score = SEARCHED_SCORE; // aka INT_MIN
    
    for (size_t i = 0; i < n_moves; i++) {
        int cur_score = score_list[i];
        
        if (cur_score > max_score) {
            max_score = cur_score;
            max_index = i;
        }
    }
    
    if (max_index != -1) {
        score_list[max_index] = SEARCHED_SCORE;
        return move_list[max_index];
    }
    
    return NO_MOVE;
}
