/* File: move.c
 * -------------
 * For more information, view "move.h".
 */

#include "move.h"

move_t encode_move(int from_sq, int to_sq, move_flag flag) {
    move_t result = 0;

    set_from(&result, from_sq);
    set_to(&result, to_sq);
    set_flag(&result, flag);

    return result;
}

int get_from(move_t move) {
    return (move & FROM_MASK) >> (SQ_MASK_LEN + FLAG_MASK_LEN);
}

int get_to(move_t move) {
    return (move & TO_MASK) >> FLAG_MASK_LEN;
}

move_flag get_flag(move_t move) {
    return (move & FLAG_MASK);
}

void set_from(move_t *move, int square) {
    *move &= ~FROM_MASK;
    *move |= square << (SQ_MASK_LEN + FLAG_MASK_LEN);
}

void set_to(move_t *move, int square) {
    *move &= ~TO_MASK;
    *move |= square << (FLAG_MASK_LEN);
}

void set_flag(move_t *move, move_flag flag) {
    *move &= ~FLAG_MASK;
    *move |= flag;
}
