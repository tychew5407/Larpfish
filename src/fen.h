/* File: fen.h
 * -------------
 * This file provides definitions for FEN encoding/parsing, used to
 * translate between FEN coded strings and board representations.
 */

#ifndef FEN_H
#define FEN_H

#include "board.h"

/* DEFINITIONS */
#define FEN_COMPONENTS 6
#define MAX_FEN_LEN 108

/* Function: parse_fen
 * --------------------
 * The `parse_fen` function takes in a pointer to a board struct and an encoded
 * FEN string and sets the board struct to align with the FEN values.
 */
void parse_fen(board *b, char *fen);

/* Function: encode_fen
 * ---------------------
 * The `encode_fen` function takes in a pointer to a board struct and modifies `fen`
 * to be its corresponding encoded FEN string.
 */
void encode_fen(board *b, char **fen);

/* Function: test_fen
 * -------------------
 * The `test_fen` function is used for debugging purposes, called via gdb. It takes in
 * a text file with FEN strings, and for each FEN string, parses it and re-encodes it
 * and compares whether the two are the same.
 */
void test_fen(char *filepath);

#endif
