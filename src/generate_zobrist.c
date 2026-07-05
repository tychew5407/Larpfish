/* File: generate_zobrist.c
 * -------------------------
 * The `generate_zobrist.c` file calls generate_zobrist_nums to
 * print out a candidate array of random numbers used for zobrist
 * hashing.
 */

#include "zobrist.h"

int main() {
    generate_zobrist_keys();
    return 0;
}
