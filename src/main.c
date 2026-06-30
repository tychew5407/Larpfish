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
#include "movegen.h"
#include "evaluation.h"
#include "search.h"

#define INIT_POS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

int main(int argc, char *argv[]) {
    char buf[MAX_FEN_LEN];
    char *fen = buf;
    board board;
    
    if (argc > 1) {
        strcpy(fen, argv[1]);
    } else {
        strcpy(fen, INIT_POS);
    }

    initialize_board(&board);
    init_attack_tables();
    parse_fen(&board, fen);

    while (true) {
        print_board(&board);
        
        move_t best_move = nega_max(&board);

        if (best_move == NO_MOVE) {
            printf("No moves in this position! Quitting.\n");
            break;
        }

        make_move(&board, best_move);
        print_board(&board);
        printf("Best move: %d, Eval: %d\n", best_move, -evaluate(&board));
        
        move_t user_move;
        printf("Enter your move (type 0 to quit): ");
        scanf("%hu", &user_move);

        if (user_move == NO_MOVE) {
            break;
        }
        make_move(&board, user_move);
    }
    
    
    return 0;
}
