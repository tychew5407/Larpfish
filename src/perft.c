/* File: perft.c
 * --------------
 * This file implements a test harness for move generation by counting all the
 * leaf nodes of a board position at a certain depth.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "bitboard.h"
#include "move.h"
#include "board.h"
#include "fen.h"
#include "move_make.h"
#include "movegen.h"

int MAX_DEPTH = 3;
const bool DIVIDED = false;

const char sqs[64][3] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

int cmp_strs(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

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

/* A variation of the perft function which enumerates the Perft of the
 * decremented depth for each move in the current position. Executes
 * when the global bool DIVIDED is true. Assumes depth >= 1.
 */
void perft_divided(board *b, int depth) {
    move_t move_list[MAX_PLY];
    size_t n_moves;
    uint64_t nodes = 0;
    
    char str_list[MAX_PLY][16];
    size_t list_len = 0;

    generate_moves(move_list, &n_moves, b);

    for (size_t i = 0; i < n_moves; i++) {
        make_move(b, move_list[i]);

        if (!is_in_check(b, b->play_side ^ 1)) {
            uint64_t move_perft = perft(b, depth - 1);
            nodes += move_perft;
            sprintf(str_list[list_len], "%s%s: %lu\n", sqs[get_from(move_list[i])], sqs[get_to(move_list[i])], move_perft);
            list_len ++;
        }

        unmake_move(b, move_list[i]);
    }

    qsort(str_list, list_len, sizeof(char) * 16, cmp_strs);

    for (size_t i = 0; i < list_len; i++) {
        printf("%s", str_list[i]);
    }

    printf("\nTotal nodes: %lu\n", nodes);
}

/* The main function runs perft on all FEN positions of given filepaths.
 */
int main(int argc, char *argv[]) {
    init_attack_tables();

    MAX_DEPTH = atoi(argv[1]);
    
    for (int i = 2; i < argc; i++) {
        FILE *FEN_file = fopen(argv[i], "r");

        if (!FEN_file) {
            printf("FAILURE: %s is an invalid filepath!\n", argv[i]);
            return 1;
        }
        
        char fen_str[MAX_FEN_LEN];
        board board;
        int positions = 1;
        uint64_t nodes = 0;
        
        while (fgets(fen_str, MAX_FEN_LEN, FEN_file) != NULL) {
            fen_str[strcspn(fen_str, "\n")] = '\0';
            initialize_board(&board);

            char *fen_ptr = fen_str;
            parse_fen(&board, fen_ptr);

            printf("Position #%d: %s\n", positions, fen_str);
            print_board(&board);
            printf("\n");
            if (DIVIDED) {
                perft_divided(&board, MAX_DEPTH);
            } else {
                for (int cur_depth = 0; cur_depth <= MAX_DEPTH; cur_depth ++) {
                    uint64_t cur_nodes = perft(&board, cur_depth);
                    nodes += cur_nodes;
                    printf("Depth = %d: %lu\n", cur_depth, cur_nodes);
                }
            }
            
            printf("\n");

            positions ++;
        }

        printf("TOTAL NODES: %lu", nodes);
    }
    
    return 0;
}
