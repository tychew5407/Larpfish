/* File: zobrist_keys.h
 * ---------------------
 * This file contains the array of randomly generated zobrist keys,
 * used for zobrist hashing.
 */

#ifndef ZOBRIST_KEYS_H
#define ZOBRIST_KEYS_H

#include <stdint.h>

/* DEFINITIONS */
#define ZOBRIST_ARR_SIZE 793

extern const uint64_t zobrist_keys[ZOBRIST_ARR_SIZE];

#endif
