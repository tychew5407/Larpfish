/* File: generate_magic.c
 * ------------------------
 * This file is responsible for generating magic numbers, used for
 * for implementing fancy magic bitboards.
 */

#include <stdio.h>
#include <stdbool.h>
#include "bitboard.h"
#include "move.h"
#include "board.h"
#include "movegen.h"

bitboard rook_attack_set[(1 << 12)];
size_t rook_set_length = 0;

static void gen_rook_attack_set(int sq, ray_dir dir, bitboard attack_bb) {
    /* if (dir == NORTH && attack_bb != 0) { */
    /*     // add to set */
    /*     for (int i = 0; i < rook_set_length; i++) { */
    /*         if (rook_attack_sets[i] == attack_bb) { */
    /*             return; */
    /*         } */
    /*     } */

    /*     rook_attack_set[rook_set_length] = attack_bb; */
    /*     rook_set_length ++; */
    /*     return; */
    /* } */
    
    /* bitboard rook_bb = 1ULL << sq; */
    /* bitboard ray_bb = ray_attacks[ROOK][dir]; */
    /* bitboard blockers_bb = rook_bb; */
    

    /* while (blockers_bb & rays[dir].wrap_check) { */
    /*     if (rays[dir].negative) { */
    /*         blockers_bb >> rays[dir].shift_amount; */
    /*     } else { */
    /*         blockers_bb << rays[dir].shift_amount; */
    /*     } */
    /* } */
}

static size_t get_bishop_attack_population(int sq) {
    return 0;
}

void get_attack_population() {
    /* for (int sq = 0; sq < NUM_SQUARES; sq ++) { */
    /*     printf("Square: %d, Rook: %lu, Bishop: %lu\n", */
    /*            sq, get_rook_attack_population(sq), get_bishop_attack_population(sq)); */
    /* } */
}

int main(int argc, char *argv[]) {
    //get_attack_population();

    return 0;
}
