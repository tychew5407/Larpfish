/* File: search.c
 * ---------------
 * For more information, see "search.h".
 */

#include <inttypes.h>
#include <stdbool.h>
#include <math.h>
#include "search.h"
#include "move_make.h"
#include "movegen.h"
#include "evaluation.h"
#include "move_ordering.h"
#include "transposition_table.h"

/* DEFINITIONS */
#define CHECKMATE_EVAL (INT16_MAX - 1)
#define ABORTED_EVAL INT16_MAX // sentinel value when search is aborted
#define NO_EVAL INT16_MIN /* sentinel value to indicate that a static evaluation was not recorded
                             on the eval stack, aka the position is in check. */

#define NO_TT_SCORE INT16_MAX  // sentinel value when TT lookup score cannot be used or is not found

#define RFP_MARGIN 150           // margin constant for reverse futility pruning
#define IMPROVING_RFP_MARGIN 135 // margin constant for RFP when position is improving
#define RFP_DEPTH_BOUND 4      // depth bound constant for reverse futility pruning

#define NMP_REDUCTION 3

#define LMR_MAX_DEPTH 64 // exclusive upper bounds
#define LMR_MAX_MOVES 64
#define LMR_DEPTH_BOUND 3

#define ASPIRATION_WINDOW_DELTA_DEFAULT 50

/* FUNCTION PROTOTYPES */
static inline move_t find_first_legal(board *b, zobrist_board game_history[]);
static int16_t alpha_beta_root(search_context *context, search_window window, move_t *best_move, uint8_t depth);
static int16_t alpha_beta(search_context *context, search_window window, int16_t eval_stack[], bool is_PV, bool allow_null_move, uint8_t depth, uint8_t ply);
static int16_t quiesce(search_context *context, search_window window, tt_node_t *node_type, uint8_t ply);
static int16_t tt_lookup(zobrist_board key, search_window *window, move_t *best_root_move, uint8_t depth);
static inline bool is_50_move_rule(board *b);
static inline int16_t no_moves_eval(bool in_check);
static inline bool is_repeat(board *b, zobrist_board game_history[]);
static inline move_t get_next_move(move_t *move_list, int *score_list, size_t n_moves);
static inline bool widen_aspiration_window(search_window *aspiration, search_window *delta, const int16_t score);
static inline void next_aspiration_window(search_window *aspiration, search_window *delta, const int16_t score);
static inline bool adjust_mate_score(int16_t *score, uint8_t ply);
static inline bool static_eval_improving(int16_t eval_stack[], uint8_t ply);
static void compute_LMR_base();

/* GLOBAL VARIABLES */
atomic_bool search_running = false;

// Lookup table for base LMR reduction w.r.t. depth and moves
static uint8_t LMR_base[LMR_MAX_DEPTH][LMR_MAX_MOVES];

void init_search_tables() {
    compute_LMR_base();
}

move_t search(search_context *context, const uint8_t max_depth) {
    if (context->n_searched) {
        *(context->n_searched) = 0;
    }

    move_t best_move = find_first_legal(context->game_board, context->game_history);
    move_t cur_move = NO_MOVE;
    int cur_depth = 1;
    
    // Aspiration window + delta bounds
    search_window aspiration_window = {.alpha = -INT16_MAX, .beta = INT16_MAX};
    search_window delta_window = {.alpha = ASPIRATION_WINDOW_DELTA_DEFAULT,
                                  .beta = ASPIRATION_WINDOW_DELTA_DEFAULT};

    // Iterative deepening
    while (cur_depth <= max_depth) {  
        int score;
        
        while (atomic_load(&search_running)) {       
            // Run until aspiration window fits true score
            score = alpha_beta_root(context, aspiration_window, &cur_move, cur_depth);

            if (!widen_aspiration_window(&aspiration_window, &delta_window, score)) {
                break;
            }
        }

        if (atomic_load(&search_running)) {
            best_move = cur_move;
            next_aspiration_window(&aspiration_window, &delta_window, score);
        } else {
            break;
        }
        
        cur_depth ++;
    }

    return best_move;
}

int16_t q_search(search_context *context) {
    if (context->n_searched) {
        *(context->n_searched) = 0;
    }

    // Throwaway argument needed, node_type is guaranteed to remain PV_NODE
    tt_node_t node_type = PV_NODE;
    
    return quiesce(context,
                   (search_window) {.alpha = -INT16_MAX, .beta = INT16_MAX},
                   &node_type,
                   0);
}

/* Function: find_first_legal
 * ---------------------------
 * The `find_first_legal` function returns the eval of the first legal move that can be
 * found, used as a last-ditch effort when the search aborts before any moves were found.
 */
static inline move_t find_first_legal(board *b, zobrist_board game_history[]) {
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

/* Function: alpha_beta_root
 * -------------------------
 * The `alpha_beta_root` function is a recursive helper function
 * of the `alpha_beta` function, called at the root level.
 */
static int16_t alpha_beta_root(search_context *context, search_window window, move_t *best_move, uint8_t depth) {
    if (!atomic_load(&search_running)) {
        return ABORTED_EVAL;
    }

    // TT lookup
    zobrist_board cur_zobrist = context->game_history[context->game_board->halfmove_clock];
    assert(cur_zobrist); // uninitialized entries are defaulted to 0
    
    int16_t tt_score = tt_lookup(cur_zobrist, &window, best_move, depth);
    if (tt_score != NO_TT_SCORE) {
        // Handle checkmate over 50-move rule
        if (!adjust_mate_score(&tt_score, 0) && is_50_move_rule(context->game_board)) {
            return 0;
        }
        
        return tt_score;
    }

    move_t move_list[MAX_MOVES];
    int score_list[MAX_MOVES];
    size_t n_moves;

    generate_moves(move_list, &n_moves, context->game_board);
    score_moves(context->game_board, context->game_history, move_list, score_list, n_moves);

    tt_node_t node_type = ALL_NODE;
    int16_t best_score = INT16_MIN;

    // Setup eval_stack for improving
    bool in_check = is_in_check(context->game_board, context->game_board->play_side);
    int16_t static_eval = evaluate(context->game_board);
    int16_t eval_stack[MAX_PLY];

    eval_stack[0] = (in_check) ? NO_EVAL : static_eval;
    
    for (size_t i = 0; i < n_moves; i++) {
        move_t cur_move = get_next_move(move_list, score_list, n_moves);
        assert(cur_move != NO_MOVE);
        
        if (context->n_searched) {
            (*(context->n_searched))++;
        }

        make_move(context->game_board, context->game_history, cur_move);

        if (!is_in_check(context->game_board, context->game_board->play_side ^ 1)) {
            int score;
            if (i == 0) {
                score = -alpha_beta(context, (search_window) {.alpha = -window.beta, .beta = -window.alpha},
                                    eval_stack, true, true, depth - 1, 1);
            } else {
                score = -alpha_beta(context, (search_window) {.alpha = -window.alpha - 1, .beta = -window.alpha},
                                    eval_stack, false, true, depth - 1, 1);

                if (score > window.alpha && score < window.beta) {
                    score = -alpha_beta(context, (search_window) {.alpha = -window.beta, .beta = -window.alpha},
                                        eval_stack, true, true, depth - 1, 1);
                }
            }

            if (score == ABORTED_EVAL || score == -ABORTED_EVAL) {
                unmake_move(context->game_board, context->game_history, cur_move);
                return ABORTED_EVAL;
            }
            
            if (score > best_score) {
                best_score = score;
                *best_move = cur_move;

                if (score > window.alpha) {
                    node_type = PV_NODE;
                    window.alpha = score;
                } else if (i == 0) {
                    // Fail-low on root PV node
                    unmake_move(context->game_board, context->game_history, cur_move);
                    return best_score;
                }
            }

            if (score >= window.beta) {
                unmake_move(context->game_board, context->game_history, cur_move);

                if (i != 0) {
                    create_tt_entry(cur_zobrist, *best_move, best_score, static_eval, depth, context->age, CUT_NODE);
                }
                
                return best_score;
            }
        }

        unmake_move(context->game_board, context->game_history, cur_move);
    }

    if (best_score == INT16_MIN) {
        int16_t no_move_score = no_moves_eval(in_check);
        //create_tt_entry(cur_zobrist, NO_MOVE, ret_score, static_eval, depth, context->age, PV_NODE);

        return no_move_score;
    }
       

    if (is_50_move_rule(context->game_board)) {
        return 0;
    }

    create_tt_entry(cur_zobrist, *best_move, best_score, static_eval, depth, context->age, node_type);
    
    return best_score;
}

/* Function: alpha_beta
 * ---------------------
 * The `alpha_beta` function is the implementation of the actual
 * alpha-beta Negamax algorithm, which is called by search().
 */
static int16_t alpha_beta(search_context *context, search_window window, int16_t eval_stack[],
                          bool is_PV, bool allow_null_move, uint8_t depth, uint8_t ply) {
    if (!atomic_load(&search_running)) {
        return ABORTED_EVAL;
    }

    if (is_repeat(context->game_board, context->game_history)) {
        return 0;
    }

    // TT lookup
    zobrist_board cur_zobrist = context->game_history[context->game_board->halfmove_clock];
    assert(cur_zobrist); // uninitialized entries are defaulted to 0
    
    int16_t tt_score = tt_lookup(cur_zobrist, &window, NULL, depth);
    if (tt_score != NO_TT_SCORE) {
        // Handle checkmate over 50-move rule
        if (!adjust_mate_score(&tt_score, ply) && is_50_move_rule(context->game_board)) {
            return 0;
        }
        
        return tt_score;
    }

    int16_t static_eval = evaluate(context->game_board);

    // Q-search
    if (depth == 0) {
        if (is_50_move_rule(context->game_board)) {
            return 0;
        }

        tt_node_t q_node_type;
        int16_t quiesce_score = quiesce(context, window, &q_node_type, ply);
        if (atomic_load(&search_running)) {
            create_tt_entry(cur_zobrist, NO_MOVE, quiesce_score, static_eval, depth, context->age, q_node_type);
        }
        
        return quiesce_score;
    }

    // Calculate improving
    bool in_check = is_in_check(context->game_board, context->game_board->play_side);
    
    eval_stack[ply] = (in_check) ? NO_EVAL : static_eval;
    
    bool improving = static_eval_improving(eval_stack, ply);
    
    // RFP
    int16_t margin = (improving) ? IMPROVING_RFP_MARGIN * depth : RFP_MARGIN * depth;

    if (depth <= RFP_DEPTH_BOUND &&
        window.beta <= INT16_MAX - margin && 
        static_eval >= window.beta + margin &&
        !is_PV &&
        !in_check /*  && */
        /* tt_move != NO_MOVE && !is_capture(tt_move) && */
        /* tt_node != PV_NODE */) {
        //create_tt_entry(cur_zobrist, NO_MOVE, static_eval, static_eval, depth, context->age, CUT_NODE);
        return static_eval;
    }

    // NMP
    if (allow_null_move && !in_check && 
        (context->game_board->piece_bbs[KNIGHT][context->game_board->play_side] ||
         context->game_board->piece_bbs[BISHOP][context->game_board->play_side] ||
         context->game_board->piece_bbs[ROOK][context->game_board->play_side] ||
         context->game_board->piece_bbs[QUEEN][context->game_board->play_side])) {
        int reduced_depth = (depth >= NMP_REDUCTION) ? depth - NMP_REDUCTION : 0;
        
        make_null_move(context->game_board, context->game_history);
        int16_t NMP_score = -alpha_beta(context, (search_window) {.alpha = -window.beta, .beta = -window.beta + 1},
                                        eval_stack, false, false, reduced_depth, ply + 1);

        unmake_null_move(context->game_board, context->game_history);
        
        if (NMP_score >= window.beta) {
            //create_tt_entry(cur_zobrist, NO_MOVE, NMP_score, static_eval, reduced_depth, context->age, CUT_NODE);
            return NMP_score;
        }
    }

    move_t move_list[MAX_MOVES];
    int score_list[MAX_MOVES];
    size_t n_moves;

    generate_moves(move_list, &n_moves, context->game_board);
    score_moves(context->game_board, context->game_history, move_list, score_list, n_moves);

    tt_node_t node_type = ALL_NODE;
    int16_t best_score = INT16_MIN;
    move_t best_move = NO_MOVE;
    
    for (size_t i = 0; i < n_moves; i++) {
        move_t cur_move = get_next_move(move_list, score_list, n_moves);
        assert(cur_move != NO_MOVE);
        
        if (context->n_searched) {
            (*(context->n_searched))++;
        }
        
        make_move(context->game_board, context->game_history, cur_move);

        if (!is_in_check(context->game_board, context->game_board->play_side ^ 1)) {
            int score;
            if (i == 0) {
                score = -alpha_beta(context, (search_window) {.alpha = -window.beta, .beta = -window.alpha},
                                    eval_stack, true, true, depth - 1, ply + 1);
            } else {
                bool should_LMR = depth >= LMR_DEPTH_BOUND;

                if (should_LMR) {
                    int LMR_depth_index = (depth < LMR_MAX_DEPTH) ? depth : LMR_MAX_DEPTH - 1;
                    int LMR_moves_index = (i < LMR_MAX_MOVES) ? i : LMR_MAX_MOVES - 1;
                    uint8_t LMR_reduction = LMR_base[LMR_depth_index][LMR_moves_index];
                    uint8_t LMR_depth = (depth > 1 + LMR_reduction) ? depth - 1 - LMR_reduction : 0;
                    
                    // Null window, reduced search
                    score = -alpha_beta(context, (search_window) {.alpha = -window.alpha - 1, .beta = -window.alpha},
                                    eval_stack, false, true, LMR_depth, ply + 1);
                }
                
                if (!should_LMR || score > window.alpha) {
                    // Null window, full search
                    score = -alpha_beta(context, (search_window) {.alpha = -window.alpha - 1, .beta = -window.alpha},
                                        eval_stack, false, true, depth - 1, ply + 1);
                }
                
                if (score > window.alpha && score < window.beta) {
                    // Full window, full search
                    score = -alpha_beta(context, (search_window) {.alpha = -window.beta, .beta = -window.alpha},
                                        eval_stack, true, true, depth - 1, ply + 1);
                }
            }

            if (score == ABORTED_EVAL || score == -ABORTED_EVAL) {
                unmake_move(context->game_board, context->game_history, cur_move);
                return ABORTED_EVAL;
            }
            
            if (score > best_score) {
                best_score = score;
                best_move = cur_move;

                if (score > window.alpha) {
                    node_type = PV_NODE;
                    window.alpha = score;
                }
            }

            if (score >= window.beta) {
                unmake_move(context->game_board, context->game_history, cur_move);

                create_tt_entry(cur_zobrist, best_move, best_score, static_eval, depth, context->age, CUT_NODE);
                
                return best_score;
            }
        }

        unmake_move(context->game_board, context->game_history, cur_move);
    }

    if (best_score == INT16_MIN) {
        int16_t no_move_score = no_moves_eval(in_check);
        create_tt_entry(cur_zobrist, NO_MOVE, no_move_score, static_eval, depth, context->age, PV_NODE);

        adjust_mate_score(&no_move_score, ply);
        return no_move_score;
    }
       

    if (is_50_move_rule(context->game_board)) {
        return 0;
    }

    create_tt_entry(cur_zobrist, best_move, best_score, static_eval, depth, context->age, node_type);
    
    return best_score;
}

/* Function: quiesce
 * ------------------
 * The `quiesce` function performs a quiesce search, performed at the leaf-nodes
 * of the alpha-beta Negamax search to avoid the horizon effect.
 */
static int16_t quiesce(search_context *context, search_window window, tt_node_t *node_type, uint8_t ply) {
    if (!atomic_load(&search_running)) {
        return ABORTED_EVAL;
    }
    
    zobrist_board cur_zobrist = context->game_history[context->game_board->halfmove_clock];
    assert(cur_zobrist);

    int16_t tt_score = tt_lookup(cur_zobrist, &window, NULL, 0);
    if (tt_score != NO_TT_SCORE) {
        adjust_mate_score(&tt_score, ply);
        return tt_score;
    }

    // Stand pat
    int16_t static_eval = evaluate(context->game_board);
    int16_t best_score = static_eval;
    
    if (best_score >= window.beta) {
        if (node_type) *node_type = CUT_NODE;
        return best_score;
    }

    if (node_type) *node_type = ALL_NODE;
    
    if (best_score > window.alpha) {
        if (node_type) *node_type = PV_NODE;
        window.alpha = best_score;
    }

    // Search
    move_t move_list[MAX_MOVES];
    int score_list[MAX_MOVES];
    size_t n_moves;
    
    generate_moves(move_list, &n_moves, context->game_board);
    score_moves(context->game_board, context->game_history, move_list, score_list, n_moves);

    for (size_t i = 0; i < n_moves; i++) {
        move_t cur_move = get_next_move(move_list, score_list, n_moves);
        assert(cur_move != NO_MOVE);

        if (!is_capture(cur_move)) {
            if (i == 0) continue;
            break;
        }
        
        if (context->n_searched) {
            (*(context->n_searched))++;
        }
        
        make_move(context->game_board, context->game_history, cur_move);

        if (!is_in_check(context->game_board, context->game_board->play_side ^ 1)) {
            int score = -quiesce(context, (search_window) {.alpha = -window.beta, .beta = -window.alpha}, NULL, ply + 1);

            if (score == ABORTED_EVAL || score == -ABORTED_EVAL) {
                unmake_move(context->game_board, context->game_history, cur_move);
                break;
            }
            
            if (score > best_score) {
                best_score = score;
                
                if (score > window.alpha) {
                    if (node_type) *node_type = PV_NODE;
                    window.alpha = score;
                }
            }

            if (score >= window.beta) {
                if (node_type) *node_type = CUT_NODE;
                unmake_move(context->game_board, context->game_history, cur_move);
                return best_score;
            }
        }

        unmake_move(context->game_board, context->game_history, cur_move);
    }

    return best_score;
}

/* Function: tt_lookup
 * --------------------
 * The `tt_lookup` function returns the score found for the position from
 * the tranposition table if it can be used, and otherwise returns NO_TT_SCORE.
 * This function adjusts alpha and beta values as needed.
 */
static int16_t tt_lookup(zobrist_board key, search_window *window, move_t *best_root_move, uint8_t depth) {
    tt_entry *cur_entry = get_tt_entry(key);
    if (in_tt(key)) {
        move_t node_move = get_tt_entry_move(*cur_entry);
        tt_node_t node_type = get_tt_entry_type(*cur_entry);

        if (get_tt_entry_depth(*cur_entry) >= depth) {
            int16_t node_score = get_tt_entry_score(*cur_entry);
            
            if (node_type == PV_NODE ||
                (node_type == CUT_NODE && node_score >= window->beta) ||
                (node_type == ALL_NODE && node_score <= window->alpha)) {
                if (best_root_move) {
                    *best_root_move = node_move;
                }
                
                return node_score;
            } else if (node_type == CUT_NODE && node_score > window->alpha && best_root_move == NULL) {
                window->alpha = node_score;
            } else if (node_type == ALL_NODE && node_score < window->beta && best_root_move == NULL) {
                window->beta = node_score;
            }
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
static inline int16_t no_moves_eval(bool in_check) {
    return (in_check) ? -CHECKMATE_EVAL : 0;
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

/* Function: widen_aspiration_window
 * -----------------------------------
 * The `adjust_aspiration_window` function returns whether the given aspiration window
 * (and deltas) needed to be adjusted based on the given score.
 */
static inline bool widen_aspiration_window(search_window *aspiration, search_window *delta, const int16_t score) {
    if (score <= aspiration->alpha) {
        aspiration->alpha = (aspiration->alpha <= -(INT16_MAX - delta->alpha)) ? -INT16_MAX : aspiration->alpha - delta->alpha;
        delta->alpha = (delta->alpha < INT16_MAX / 2) ? delta->alpha * 2 : INT16_MAX;
        return true;
    }

    if (score >= aspiration->beta) {
        aspiration->beta = (aspiration->beta >= INT16_MAX - delta->beta) ? INT16_MAX : aspiration->beta + delta->beta;
        delta->beta = (delta->beta < INT16_MAX / 2) ? delta->beta * 2 : INT16_MAX;
        return true;
    }

    return false;
}

/* Function: next_aspiration_window
 * ---------------------------------
 * The `next_aspiration_window` function sets the values of the aspiration and delta
 * search windows accordingly to the score found on the current iteration. To be called
 * on setting up for the next iteration of iterative deepening.
 */
static inline void next_aspiration_window(search_window *aspiration, search_window *delta, const int16_t score) {
    (*delta) = (search_window) {
        .alpha = ASPIRATION_WINDOW_DELTA_DEFAULT,
        .beta = ASPIRATION_WINDOW_DELTA_DEFAULT
    };

    (*aspiration) = (search_window) {
        .alpha = (score >= -INT16_MAX + delta->alpha) ? score - delta->alpha : -INT16_MAX,
        .beta  = (score <= INT16_MAX - delta->beta)   ? score + delta->beta  : INT16_MAX
    };
}

/* Function: adjust_mate_score
 * ----------------------------
 * The `adjust_mate_score` function adjusts a given mate score according to the given
 * ply and returns true. If the given score is not a mate score, then it returns false.
 */
static inline bool adjust_mate_score(int16_t *score, uint8_t ply) {
    if (*score == CHECKMATE_EVAL) {
        (*score) -= ply;
        return true;
    }

    if (*score == -CHECKMATE_EVAL) {
        (*score) += ply;
        return true;
    }
    
    return false;
}

/* Function: static_eval_improving
 * --------------------------------
 * The `static_eval_improving` function returns a boolean for whether the current
 * searched position has a static evaluation that has improved from two plies or
 * four plies ago. Positions that are in check are skipped. This result is an important
 * modifer used for search heuristics.
 */
static inline bool static_eval_improving(int16_t eval_stack[], uint8_t ply) {
    if (eval_stack[ply] == NO_EVAL) {
        // Current position is in check
        return false;
    }

    if (ply >= 2 && eval_stack[ply - 2] != NO_EVAL) {
        return eval_stack[ply] > eval_stack[ply - 2];
    }

    if (ply >= 4 && eval_stack[ply - 4] != NO_EVAL) {
        return eval_stack[ply] > eval_stack[ply - 4];
    }

    return true;
}

/* Function: compute_LMR_base
 * ---------------------------
 * The `compute_LMR_base` function populates the precomputed lookup table for
 * LMR base reduction values, using the formula from Obsidian:
 *
 * = 0.99 + ln(depth) * ln(moves) / 3.14
 */
static void compute_LMR_base() {
    for (int depth = 1; depth < LMR_MAX_DEPTH; depth++) {
        for (int moves = 1; moves < LMR_MAX_MOVES; moves++) {
            LMR_base[depth][moves] = 0.99 + log(depth) * log(moves) / 3.14;
        }
    }
}
