/* File: generate_attack_tables.c
 * --------------------------------
 * This file is used for creating/precomputing and printing candidate
 * attack tables to use for pseudo-legal move generation.
 *
 * Different flags are used to generate different attacks tables:
 *   -p: pawn attack table
 *   -n: knight attack table
 *   -k: king attack table
 *
 * For simplicity, this program only processes the first flag.
 */

#include <stdio.h>
#include <string.h>
#include "attack_tables.h"

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Specify an attack table to generate!");
        return 1;
    }

    if (!strcmp(argv[1], "-p")) {
        generate_pawn_attacks();
    } else if (!strcmp(argv[1], "-n")) {
        generate_knight_attacks();
    } else if (!strcmp(argv[1], "-k")) {
        generate_king_attacks();
    } else {
        printf("Invalid flag!");
        return 1;
    }

    return 0;
}
