/* File: perft.c
 * --------------
 * This file implements a test harness for move generation by counting all the
 * leaf nodes of a board position at a certain depth.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "bitboard.h"
#include "move.h"
#include "board.h"
#include "fen.h"
#include "move_make.h"
#include "movegen.h"

const int MAX_DEPTH = 3;

/* The perft function, which counts all nodes (legal moves) of a given board
 * position at a certain depth. Assumes that depth >= 1 and that the board
 * posiiton is valid (does not break the standard rules of the game).
 */
uint64_t perft(board *b, int depth) {
    move_t move_list[MAX_PLY];
    size_t n_moves;
    uint64_t nodes = 0;

    if (depth == 0) {
        return 1;
    }

    generate_moves(move_list, &n_moves, b);
    
    for (size_t i = 0; i < n_moves; i++) {
        /* board orig = *b; */
        make_move(b, move_list[i]);

        if (!is_in_check(b, b->play_side ^ 1)) {
            nodes += perft(b, depth - 1);
        }

        unmake_move(b, move_list[i]);

        /* if (!board_cmp(orig, *b)) { */
        /*     printf("Unmake move failed!\n"); */
        /* } */
    }

    return nodes;
}

/* The main function runs perft on all FEN positions of given filepaths.
 */
int main(int argc, char *argv[]) {
    init_attack_tables();

    for (int i = 1; i < argc; i++) {
        FILE *FEN_file = fopen(argv[i], "r");

        if (!FEN_file) {
            printf("FAILURE: %s is an invalid filepath!\n", argv[i]);
            return 1;
        }
        
        char fen_str[MAX_FEN_LEN];
        board board;
        int positions = 1;
        
        while (fgets(fen_str, MAX_FEN_LEN, FEN_file) != NULL) {
            fen_str[strcspn(fen_str, "\n")] = '\0';
            initialize_board(&board);

            char *fen_ptr = fen_str;
            parse_fen(&board, fen_ptr);

            printf("Position #%d: %s\n", positions, fen_str);
            print_board(&board);
            printf("\n");
            for (int cur_depth = 0; cur_depth <= MAX_DEPTH; cur_depth ++) {
                printf("Depth = %d: %lu\n", cur_depth, perft(&board, cur_depth));
            }
            printf("\n");

            positions ++;
        }        
    }
    
    return 0;
}
