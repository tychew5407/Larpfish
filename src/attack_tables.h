/* File: attack_tables.h
 * ----------------------
 * This file contains attack tables for efficient move generation.
 * This project uses fancy magic bitboards for sliding moves.
 */

#ifndef ATTACK_TABLES_H
#define ATTACK_TABLES_H

#include "definitions.h"
#include "bitboard.h"

/* DEFINITIONS */
#define KNIGHT_DIRS 8
#define KING_DIRS 8
#define RAY_DIRS 8
#define NUM_ROOK_RAYS 4
#define NUM_BISHOP_RAYS 4
#define NUM_SLIDERS 3
#define NUM_SLIDING_ATTACKS 107648

typedef struct {
    bitboard occ_mask;
    bitboard *attack_table;
    uint64_t magic_num;
    size_t shift;
} magic;

/* ATTACK TABLES */
extern bitboard pawn_attacks[NUM_SIDES][NUM_SQUARES];
extern bitboard knight_attacks[NUM_SQUARES];
extern bitboard king_attacks[NUM_SQUARES];
extern magic rook_magics[NUM_SQUARES];
extern magic bishop_magics[NUM_SQUARES];

/* Function: init_attack_tables
 * -----------------------------
 * The `init_attack_tables` function computes and sets
 * all of the above corresponding attack/magic tables.
 * To be called on initialization of the engine.
 */
void init_attack_tables();

/* Function: free_sliding_attacks
 * ------------------------------
 * The `free_sliding_attacks` function frees the internal
 * heap-allocated sliding attacks table. To be called upon
 * exit of the engine program.
 */
void free_sliding_attacks();

#endif
