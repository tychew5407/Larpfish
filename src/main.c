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

#define INIT_POS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

int main(int argc, char *argv[]) {
    char buf[MAX_FEN_LEN];
    char *fen = buf;
    board board;
    move_t move_list[MAX_MOVES];
    size_t move_list_len;
    
    if (argc > 1) {
        strcpy(fen, argv[1]);
    } else {
        strcpy(fen, INIT_POS);
    }
    
    initialize_board(&board);
    init_attack_tables();
    parse_fen(&board, fen);
    print_board(&board);
    generate_moves(move_list, &move_list_len, &board);
    
    return 0;
}
