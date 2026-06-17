/* File: main.c
 * -------------
 * This is the main global C file that will contain the main function to be executed.
 */

#include "board.h"

#define INIT_POS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

int main(int argc, char *argv[]) {
    char *fen;
    board board;
    
    if (argc > 1) {
        fen = argv[1];
    } else {
        fen = INIT_POS;
    }
    
    initialize_board(&board);
    parse_fen(&board, fen);
    print_board(&board);

    return 0;
}
