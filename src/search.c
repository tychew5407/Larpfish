/* File: search.c
 * ---------------
 * For more information, see "search.h".
 */

#include <inttypes.h>
#include <stdbool.h>
#include "search.h"
#include "move_ordering.h"
#include "transposition_table.h"

atomic_bool search_running = false;

/* FUNCTION PROTOTYPES */
static inline bool is_50_move_rule(board *b);
static inline int16_t no_moves_eval(board *b);
static inline bool is_repeat(board *b, zobrist_board game_history[]);
static inline move_t get_next_move(move_t *move_list, int *score_list, size_t n_moves);
static int16_t alpha_beta(board *b, zobrist_board game_history[], move_t *best_root_move, uint64_t *n_searched, int16_t alpha, int16_t beta, uint8_t depth, uint8_t age);
static int16_t quiesce(board *b, zobrist_board game_history[], uint64_t *n_searched, int16_t alpha, int16_t beta, tt_node_t *node_t);
static int16_t tt_lookup(move_t *best_root_move, zobrist_board zb, int16_t *alpha, int16_t *beta, uint8_t depth);

int search(board *b, zobrist_board game_history[], move_t *best_move, uint64_t *n_searched, uint8_t depth, uint8_t age, int16_t alpha, int16_t beta) {
    if (n_searched) {
        *n_searched = 0;
    }

    int score = alpha_beta(b, game_history, best_move, n_searched, alpha, beta, depth, age);

    return score;
}

move_t find_first_legal(board *b, zobrist_board game_history[]) {
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
static int16_t alpha_beta(board *b, zobrist_board game_history[], move_t *best_root_move, uint64_t *n_searched, int16_t alpha, int16_t beta, uint8_t depth, uint8_t age) {
    if (!atomic_load(&search_running)) {
        return ABORTED_EVAL;
    }

    if (is_repeat(b, game_history)) {
        return 0;
    }

    zobrist_board cur_zobrist = game_history[b->halfmove_clock];
    assert(cur_zobrist);
    int16_t tt_score = tt_lookup(best_root_move, cur_zobrist, &alpha, &beta, depth);
    if (tt_score != NO_TT_SCORE) {
        // Handle checkmate over 50-move rule
        if (tt_score == CHECKMATE_EVAL) {
            tt_score -= b->fullmove_counter;
        } else if (tt_score == -CHECKMATE_EVAL) {
            tt_score += b->fullmove_counter;
        } else if (is_50_move_rule(b)) {
            return 0;
        }
        
        return tt_score;
    }

    int16_t static_eval = evaluate(b);
    
    if (depth == 0) {
        if (is_50_move_rule(b)) {
            return 0;
        }

        tt_node_t q_node_type;
        int16_t ret_score = quiesce(b, game_history, n_searched, alpha, beta, &q_node_type);
        if (atomic_load(&search_running)) create_tt_entry(cur_zobrist, NO_MOVE, ret_score, static_eval, depth, age, q_node_type);
        return ret_score;
    }

    move_t move_list[MAX_MOVES];
    int score_list[MAX_MOVES];
    size_t n_moves;

    generate_moves(move_list, &n_moves, b);
    score_moves(b, game_history, move_list, score_list, n_moves);

    tt_node_t cur_tt_type = ALL_NODE;
    int16_t best_score = INT16_MIN;
    move_t best_move = NO_MOVE;
    
    for (size_t i = 0; i < n_moves; i++) {
        move_t cur_move = get_next_move(move_list, score_list, n_moves);
        assert(cur_move != NO_MOVE);
        
        if (n_searched) {
            (*n_searched)++;
        }
        
        make_move(b, game_history, cur_move);

        if (!is_in_check(b, b->play_side ^ 1)) {
            int score;
            if (i == 0) {
                score = -alpha_beta(b, game_history, NULL, n_searched, -beta, -alpha, depth - 1, age);
            } else {
                score = -alpha_beta(b, game_history, NULL, n_searched, -alpha - 1, -alpha, depth - 1, age);

                if (score > alpha && score < beta) {
                    score = -alpha_beta(b, game_history, NULL, n_searched, -beta, -alpha, depth - 1, age);
                }
            }

            if (score == ABORTED_EVAL || score == -ABORTED_EVAL) {
                unmake_move(b, game_history, cur_move);
                return ABORTED_EVAL;
            }
            
            if (score > best_score) {
                best_score = score;
                best_move = cur_move;

                if (score > alpha) {
                    cur_tt_type = PV_NODE;
                    alpha = score;
                } else if (i == 0 && best_root_move) {
                    /* Possible with aspiration windows, need to discard result entirely,
                     * notably early and without creating a TT entry.
                     */
                    unmake_move(b, game_history, cur_move);
                    return best_score;
                }
            }

            if (score >= beta) {
                unmake_move(b, game_history, cur_move);

                if (i != 0 && best_root_move == NULL) {
                    create_tt_entry(cur_zobrist, best_move, best_score, static_eval, depth, age, CUT_NODE);
                }
                
                return best_score;
            }
        }

        unmake_move(b, game_history, cur_move);
    }

    if (best_score == INT16_MIN) {
        int16_t ret_score = no_moves_eval(b);
        create_tt_entry(cur_zobrist, NO_MOVE, ret_score, static_eval, depth, age, PV_NODE);

        if (ret_score != 0) ret_score += b->fullmove_counter;
        return ret_score;
    }
       

    if (is_50_move_rule(b)) {
        return 0;
    }

    if (best_root_move) {
        *best_root_move = best_move;
    }

    create_tt_entry(cur_zobrist, best_move, best_score, static_eval, depth, age, cur_tt_type);
    
    return best_score;
}

/* Function: quiesce
 * ------------------
 * The `quiesce` function performs a quiesce search, performed at the leaf-nodes
 * of the alpha-beta Negamax search to avoid the horizon effect.
 */
static int16_t quiesce(board *b, zobrist_board game_history[], uint64_t *n_searched, int16_t alpha, int16_t beta, tt_node_t *node_t) {
    if (!atomic_load(&search_running)) {
        return ABORTED_EVAL;
    }
    
    zobrist_board cur_zobrist = game_history[b->halfmove_clock];
    assert(cur_zobrist);
    int16_t tt_score = tt_lookup(NULL, cur_zobrist, &alpha, &beta, 0);
    if (tt_score != NO_TT_SCORE) {
        if (tt_score == CHECKMATE_EVAL) {
            tt_score -= b->fullmove_counter;
        } else if (tt_score == -CHECKMATE_EVAL) {
            tt_score += b->fullmove_counter;
        }
        
        return tt_score;
    }
    
    int16_t static_eval = evaluate(b);
    
    move_t move_list[MAX_MOVES];
    int score_list[MAX_MOVES];
    size_t n_moves;

    // Stand pat
    int16_t best_score = static_eval;
    
    if (best_score >= beta) {
        if (node_t) *node_t = CUT_NODE;
        return best_score;
    }

    if (node_t) *node_t = ALL_NODE;
    
    if (best_score > alpha) {
        if (node_t) *node_t = PV_NODE;
        alpha = best_score;
    }

    generate_moves(move_list, &n_moves, b);
    score_moves(b, game_history, move_list, score_list, n_moves);

    for (size_t i = 0; i < n_moves; i++) {
        move_t cur_move = get_next_move(move_list, score_list, n_moves);
        assert(cur_move != NO_MOVE);

        if (!(get_flag(cur_move) & CAPTURE_FLAG)) break;
        
        if (n_searched) {
            (*n_searched)++;
        }
        
        make_move(b, game_history, cur_move);

        if (!is_in_check(b, b->play_side ^ 1)) {
            int score = -quiesce(b, game_history, n_searched, -beta, -alpha, NULL);

            if (score == ABORTED_EVAL || score == -ABORTED_EVAL) {
                unmake_move(b, game_history, cur_move);
                break;
            }
            
            if (score > best_score) {
                best_score = score;
                
                if (score > alpha) {
                    if (node_t) *node_t = PV_NODE;
                    alpha = score;
                }
            }

            if (score >= beta) {
                if (node_t) *node_t = CUT_NODE;
                unmake_move(b, game_history, cur_move);
                return best_score;
            }
        }

        unmake_move(b, game_history, cur_move);
    }

    return best_score;
}

/* Function: tt_lookup
 * --------------------
 * The `tt_lookup` function returns the score found for the position from
 * the tranposition table if it can be used, and otherwise returns NO_TT_SCORE.
 * This function adjusts alpha and beta values as needed.
 */
static int16_t tt_lookup(move_t *best_root_move, zobrist_board zb, int16_t *alpha, int16_t *beta, uint8_t depth) {
    tt_entry *cur_entry = get_tt_entry(zb);
    if (in_tt(zb) &&
        get_tt_entry_depth(*cur_entry) >= depth) {
        tt_node_t node_type = get_tt_entry_type(*cur_entry);
        int16_t node_score = get_tt_entry_score(*cur_entry);

        if (node_type == PV_NODE ||
            (node_type == CUT_NODE && node_score >= *beta) ||
            (node_type == ALL_NODE && node_score <= *alpha)) {
            if (best_root_move) {
                *best_root_move = get_tt_entry_move(*cur_entry);
            }

            return node_score;
        } else if (node_type == CUT_NODE && node_score > *alpha && best_root_move == NULL) {
            *alpha = node_score;
        } else if (node_type == ALL_NODE && node_score < *beta && best_root_move == NULL) {
            *beta = node_score;
        }
    }

    return NO_TT_SCORE;
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
static inline int16_t no_moves_eval(board *b) {
    return (is_in_check(b, b->play_side)) ? -CHECKMATE_EVAL : 0;
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
