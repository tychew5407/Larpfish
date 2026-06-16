/* File: main.c
 * -------------
 * This is the main global C file that will contain the main function to be executed.
 */

#include "board.h"

int main(int argc, char *argv[]) {
    char *fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    board board;
    initialize_board(&board);
    parse_fen(&board, fen);
    print_board(&board);
    return 0;
}
