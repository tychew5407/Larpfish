/* File: history.c
 * ----------------
 * The `history.c` file implements the history table and
 * related updating methods for it.
 *
 * For more information, see "history.h".
 */

#include <inttypes.h>
#include "history.h"
#include "definitions.h"

/* GLOBAL VARIABLES */
int16_t history_table[NUM_SIDES][NUM_SQUARES][NUM_SQUARES];

void init_history_table() {
    for (int i = 0; i < NUM_SIDES; i++) {
        for (int j = 0; j < NUM_SQUARES; j++) {
            for (int k = 0; k < NUM_SQUARES; k++) {
                history_table[i][j][k] = 0;
            }
        }
    }
}
