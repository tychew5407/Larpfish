/* File: test_search.c
 * --------------------
 * This file implements a test harness for search by printing out
 * the best move and corresponding eval from the search algorithm
 * given board positions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "definitions.h"
#include "bitboard.h"
#include "move.h"
#include "board.h"
#include "fen.h"
#include "zobrist.h"
#include "move_make.h"
#include "movegen.h"
#include "search.h"

static int MAX_DEPTH = 4;

int main(int argc, char *argv[]) {
    init_attack_tables();

    char *file_path = argv[1];
    
    if (argc > 2) {
        MAX_DEPTH = atoi(argv[1]);
        file_path = argv[2];
    }

    FILE *FEN_file = fopen(file_path, "r");

    if (!FEN_file) {
        printf("FAILURE: %s is an invalid filepath!\n", file_path);
        return 1;
    }

    char fen_str[MAX_FEN_LEN];
    board board;
    zobrist_board game_history[MAX_HALFMOVES + MAX_DEPTH];

    for (int i = 0; i < MAX_HALFMOVES + MAX_DEPTH; i++) {
        game_history[i] = 0;
    }
    
    int positions = 1;

    while (fgets(fen_str, MAX_FEN_LEN, FEN_file) != NULL) {
        fen_str[strcspn(fen_str, "\n")] = '\0';
        initialize_board(&board);

        char *fen_ptr = fen_str;
        parse_fen(&board, fen_ptr);
        game_history[board.halfmove_clock] = generate_zobrist_board(&board);

        printf("Position #%d: %s\n", positions, fen_str);
        print_board(&board);
        printf("\n");

        atomic_store(&search_running, true);
        
        move_t best_move = NO_MOVE;
        uint64_t nodes_searched = 0;
        int best_eval = search(&board, game_history, &best_move, &nodes_searched, MAX_DEPTH, 0);
        
        atomic_store(&search_running, false);

        if (best_move != NO_MOVE) {
            printf("Best move: %d to %d. Eval: %d\n", get_from(best_move), get_to(best_move), best_eval);
        } else {
            printf("No moves in this position. Eval: %d\n", best_eval);
        }

        printf("Nodes searched: %" PRIu64 "\n", nodes_searched);
        printf("\n");
        
        positions ++;
    }
}
