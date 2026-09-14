/* File: history.h
 * ----------------
 * The `history.h` file defines the history table
 * used for move ordering purposes via the history
 * heuristic.
 */

#ifndef HISTORY_H
#define HISTORY_H

#include <inttypes.h>
#include "definitions.h"
#include "move.h"

/* DEFINITIONS */
#define MAX_HISTORY_SCORE INT16_MAX

/* The history table (or butterfly boards) is indexed
 * via (side, from sq, to sq) in which the indices
 * represent moves, and each element is a counter that
 * keep tracks of the number of times a particular move
 * resulted in a beta-cutoff.
 */
extern int16_t history_table[NUM_SIDES][NUM_SQUARES][NUM_SQUARES];

/* Function: init_history_table
 * -----------------------------
 * The `init_history_table` function zeroes out all of the
 * entries in the history table. Should be called upon every
 * new game.
 */
void init_history_table();

/* Function: update_history_table
 * -------------------------------
 * The `update_history_table` function increments the
 * corresponding history table entry according to the
 * history gravity formula.
 */
static inline void update_history_table(const side side, const move_t move, const int32_t bonus) {
    int32_t clamped_bonus;

    if (bonus < -MAX_HISTORY_SCORE) {
        clamped_bonus = -MAX_HISTORY_SCORE;
    } else if (bonus > MAX_HISTORY_SCORE) {
        clamped_bonus = MAX_HISTORY_SCORE;
    } else {
        clamped_bonus = bonus;
    }

    int32_t abs_bonus = (clamped_bonus < 0) ? -clamped_bonus : clamped_bonus;
    
    // History gravity formula
    history_table[side][get_from(move)][get_to(move)] += clamped_bonus -
        history_table[side][get_from(move)][get_to(move)] * abs_bonus / MAX_HISTORY_SCORE;
}

/* Function: get_history_score
 * ----------------------------
 * The `get_history_score` function outputs the score
 * of the corresponding history table entry.
 */
static inline int16_t get_history_score(const side side, const move_t move) {
    return history_table[side][get_from(move)][get_to(move)];
}

#endif
