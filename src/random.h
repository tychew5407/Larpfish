/* File: random.h
 * ---------------
 * The `random.h` file implements a quick pseudo-random number generator
 * (PRNG) for the purposes of Zobrist hashing and generating magic bitboards.
 *
 * This project uses the Xorshift method for generating random numbers. The
 * rand() function from standard C libraries have issues with quality of
 * randomness, period, and bit-width.
 */

#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

typedef struct {
    uint64_t a;
} xorshift64_state;

typedef struct {
	uint64_t s;
} mix64_state;

static inline uint64_t xorshift64(xorshift64_state *state) {
    uint64_t x = state->a;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	return state->a = x;
}

/* Initializing states uses a SplitMix64 generator, which is recommended
 * to avoid all-zero sequences.
 */
static inline uint64_t mix64(mix64_state* state) {
	uint64_t result = (state->s += 0x9E3779B97F4A7C15);
	result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
	result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
	return result ^ (result >> 31);
}

static inline void xorshift64_init(xorshift64_state *state, uint64_t seed) {
    mix64_state smstate = { seed };
    
    state->a = mix64(&smstate);
}

#endif
