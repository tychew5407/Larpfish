/* File: main.c
 * -------------
 * This is the main global C file that will contain the main function to be executed.
 */

#include <string.h>
#include <stdio.h>
#include "definitions.h"
#include "move.h"
#include "board.h"
#include "fen.h"
#include "zobrist.h"
#include "move_make.h"
#include "movegen.h"
#include "evaluation.h"
#include "search.h"

#define INIT_POS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

const int SEARCH_DEPTH = 4;

int main(int argc, char *argv[]) {
    char buf[MAX_FEN_LEN];
    char *fen = buf;
    board board;
    zobrist_board game_history[MAX_HALFMOVES + SEARCH_DEPTH];
    
    if (argc > 1) {
        strcpy(fen, argv[1]);
    } else {
        strcpy(fen, INIT_POS);
    }

    initialize_board(&board);
    init_attack_tables();
    parse_fen(&board, fen);
    game_history[0] = generate_zobrist_board(&board);

    bool user = false;
    if (argc > 2 && !strcmp(argv[2], "-u")) user = true;

    while (true) {
        print_board(&board);
        printf("\n");
        
        move_t best_move = NO_MOVE;
        int best_eval = nega_max(&board, game_history, &best_move, SEARCH_DEPTH);

        if (best_move == NO_MOVE) {
            printf("No moves in this position! Quitting.\n");
            break;
        }

        make_move(&board, game_history, best_move);
        print_board(&board);
        printf("Best move: %d, Eval: %d\n\n", best_move, best_eval);

        if (user) {
            move_t user_move;
            printf("Enter your move (type 0 to quit): ");
            scanf("%hu", &user_move);

            if (user_move == NO_MOVE) {
                break;
            }
            make_move(&board, game_history, user_move);
        }
    }
    
    
    return 0;
}
