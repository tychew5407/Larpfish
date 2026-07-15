/* File: attack_tables.h
 * ----------------------
 * This file contains the precomputed attack tables and magic numbers
 * used for efficient move making, as well as functions for printing
 * out candidate attack tables.
 */

#ifndef ATTACK_TABLES_H
#define ATTACK_TABLES_H

#include "definitions.h"
#include "bitboard.h"

/* DEFINITIONS */
#define KNIGHT_DIRS 8
#define KING_DIRS 8

/* ATTACK TABLES */
extern const bitboard pawn_attacks[NUM_SQUARES][NUM_SIDES];
extern const bitboard knight_attacks[NUM_SQUARES];
extern const bitboard king_attacks[NUM_SQUARES];

/* Function: generate_pawn_attacks
 * --------------------------------
 * The `generate_pawn_attacks` function computes and prints
 * out the pawn_attacks table.
 */
void generate_pawn_attacks();

/* Function: generate_knight_attacks
 * ----------------------------------
 * The `generate_knight_attacks` function computes and prints
 * out the knight_attacks table.
 */
void generate_knight_attacks();

/* Function: generate_king_attacks
 * --------------------------------
 * The `generate_king_attacks` function computes and prints
 * out the king_attacks table.
 */
void generate_king_attacks();

#endif
