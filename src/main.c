/* File: main.c
 * -------------
 * This is the main global C file that will contain the main function to be executed.
 * Handles all UCI-support.
 */

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <inttypes.h>
#include "definitions.h"
#include "move.h"
#include "board.h"
#include "board_ascii.h"
#include "fen.h"
#include "zobrist.h"
#include "transposition_table.h"
#include "move_make.h"
#include "attack_tables.h"
#include "movegen.h"
#include "evaluation.h"
#include "search.h"

/* ENGINE DEFINITIONS */
#define ENGINE_NAME "Larpfish 0.7"
#define ENGINE_AUTHOR "Tyler Chew"
#define START_POS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define DEFAULT_TT_SIZE 128
#define MIN_HASH_SIZE 1
#define MAX_HASH_SIZE 512
#define MAX_DEPTH 100
#define INFINITE_SEARCH_TIME 0
#define TIMER_INTERVAL 10

/* UCI PROTOCOL DEFINITIONS */
#define UCI_BUF_SIZE (128 * 1024)
#define NUM_SUPPORTED_CMDS 8
#define NUM_SUPPORTED_OPTION_CMDS 1
#define NULL_MOVE "0000"

/* The `cmd` struct represents a UCI command from the GUI, and has two fields:
 *   - `cmd_text`: the input text corresponding to the command
 *   - `cmd_func`: the function to be called corresponding to the command
 */
typedef struct {
    char *cmd_text;
    void (*cmd_func)(char *);
} cmd;

/* ENGINE FUNCTION PROTOTYPES */
static void initialize_game();

/* UCI FUNCTION PROTOTYPES */
static void handle_uci(char *args);
static void handle_isready(char *args);
static void handle_setoption(char *args);
static void handle_ucinewgame(char *args);
static void handle_position(char *args);
static void handle_go(char *args);
static void handle_stop(char *args);
static void handle_quit(char *args);
static void set_hash_size(char *val);
static void encode_UCI(move_t move, char *UCI_str);
static move_t decode_UCI(const board *b, const char *UCI_str);
static void *search_helper(void *arg);
static void *search_timer(void *arg);

/* ENGINE GLOBAL VARIABLES:
 *   - `game_board`: the board representation of the current game.
 *   - `game_history`: the game history from the last irreversible move.
 *   - `TT_size_MB`: the size of the engine's transposition hash table, in megabytes.
 */
static board game_board;
static zobrist_board game_history[MAX_HALFMOVES + MAX_DEPTH];

static size_t TT_size_MB = DEFAULT_TT_SIZE;
static uint8_t search_age = 0;

/* UCI PROTOCOL GLOBAL VARIABLES:
 *   - `running` specifies whether the engine should continue running or not
 *     (i.e. process UCI commands from the GUI)
 *   - `cmd_input` is the string buffer holding the UCI commands from the GUI.
 *   - `commands` is a list of engine-supported UCI commands that uses `cmd`
 *     structs to pair text inputs to functions.
 *   - `option_subcmds` is a list of engine-supported UCI subcommands from the
 *     `setoption` command.
 */
static bool running = true;
static char cmd_input[UCI_BUF_SIZE];

static cmd commands[NUM_SUPPORTED_CMDS] = {
    {"uci", handle_uci},
    {"isready", handle_isready},
    {"setoption", handle_setoption},
    {"ucinewgame", handle_ucinewgame},
    {"position", handle_position},
    {"go", handle_go},
    {"stop", handle_stop},
    {"quit", handle_quit}
};

static cmd option_subcmds[NUM_SUPPORTED_OPTION_CMDS] = {
    {"Hash", set_hash_size}
};

static pthread_t search_thread;
static pthread_t timer_thread;
static atomic_bool timer_running = false;
static uint64_t timer_duration = INFINITE_SEARCH_TIME;
static bool search_thread_live = false;
static bool timer_thread_live = false;

int main() {
    init_attack_tables();
    init_search_tables();
    
    while (running && fgets(cmd_input, UCI_BUF_SIZE, stdin)) {
        /* Truncate the newline character, or raise an error if no newline
         * is found (meaning that the command exceeded the buffer size).
         */
        if (cmd_input[strlen(cmd_input) - 1] != '\n') {
            return 1;
        }

        cmd_input[strlen(cmd_input) - 1] = '\0';

        // Parse command + flags
        char *cur_args = strchr(cmd_input, ' ');
        if (cur_args) cur_args++;
        
        char *command = strtok(cmd_input, " ");
        
        for (int i = 0; i < NUM_SUPPORTED_CMDS; i++) {
            if (!strcmp(command, commands[i].cmd_text)) {
                commands[i].cmd_func(cur_args);
                break;
            }
        }
    }
    
    return 0;
}

static void initialize_game() {
    handle_stop(NULL);
    initialize_board(&game_board);

    for (size_t i = 0; i < sizeof(game_history) / sizeof(zobrist_board); i++) {
        game_history[i] = 0;
    }

    empty_move_stack();
}

static void handle_uci(char *args) {
    // Identify engine
    printf("id name %s\n", ENGINE_NAME);
    printf("id author %s\n", ENGINE_AUTHOR);

    // Send options
    printf("option name Hash type spin default %d min %d max %d\n", MIN_HASH_SIZE, MIN_HASH_SIZE, MAX_HASH_SIZE);
    
    // Confirm
    printf("uciok\n");

    fflush(stdout);
}

static void handle_isready(char *args) {
    printf("readyok\n");
    fflush(stdout);
}

static void handle_ucinewgame(char *args) {
    initialize_game();

    if (tt_exists()) {
        free_tt();
    }

    init_tt(TT_size_MB);
    search_age = 0;
}

static void handle_setoption(char *args) {
    assert(args);

    char *cmd_name = strstr(args, "name ");
    assert(cmd_name);
    cmd_name += 5;

    size_t name_len = strlen(cmd_name);
    
    char *cmd_val = strstr(args, " value ");
    if (cmd_val) {
        name_len -= strlen(cmd_val);
        cmd_val += 6;
    }

    for (int i = 0; i < NUM_SUPPORTED_OPTION_CMDS; i++) {
        if (!strncmp(cmd_name, option_subcmds[i].cmd_text, name_len)) {
            option_subcmds[i].cmd_func(cmd_val);
            return;
        }
    }
}

static void set_hash_size(char *val) {
    TT_size_MB = atoi(val);
    assert(TT_size_MB);
}

static void handle_position(char *args) {
    if (!args) {
        return;
    }

    initialize_game();

    /* Process arguments, assumes a valid command and
     * initialized board.
     */
    char *moves_ptr = strstr(args, " moves");

    if (moves_ptr) {
        *moves_ptr = '\0';
        moves_ptr ++;
    }
    
    char fen_buf[MAX_FEN_LEN];
    char *fen_ptr = fen_buf;
    
    if (!strncmp(args, "startpos", 8)) {
        strcpy(fen_buf, START_POS);
    } else {
        strcpy(fen_buf, args + strlen("fen "));
    }

    parse_fen(&game_board, fen_ptr);
    game_history[game_board.halfmove_clock] = generate_zobrist_board(&game_board);

    moves_ptr = strtok(moves_ptr, " ");

    if (moves_ptr) {
        moves_ptr = strtok(NULL, " ");
    }
    
    while (moves_ptr) {
        move_t move = decode_UCI(&game_board, moves_ptr);
        make_move(&game_board, game_history, move);
        empty_move_stack();
        moves_ptr = strtok(NULL, " ");
    }
}

static void handle_go(char *args) {
    handle_stop(args); // To clean up any running threads

    // Parse time subcommands
    timer_duration = INFINITE_SEARCH_TIME;
    char time_str[6];
    char inc_str[5];

    sprintf(time_str, "%ctime", SIDE_ASCII[game_board.play_side]);
    sprintf(inc_str, "%cinc", SIDE_ASCII[game_board.play_side]);
    
    args = strtok(args, " ");
    while (args) {
        if (!strcmp(args, "movetime")) {
            args = strtok(NULL, " ");
            timer_duration = atoi(args);
            break;
        } else if (!strcmp(args, time_str)) {
            args = strtok(NULL, " ");
            timer_duration += atoi(args) / 20;
        } else if (!strcmp(args, inc_str)) {
            args = strtok(NULL, " ");
            timer_duration += atoi(args) / 2;
        }
        
        args = strtok(NULL, " ");
    }

    search_thread_live = true;
    atomic_store(&search_running, true);
    pthread_create(&search_thread, NULL, search_helper, NULL);
    
    if (timer_duration != INFINITE_SEARCH_TIME) {
        timer_thread_live = true;
        atomic_store(&timer_running, true);
        pthread_create(&timer_thread, NULL, search_timer, NULL);
    }
    
}

static void handle_stop(char *args) {
    if (search_thread_live) {
        search_thread_live = false;
        atomic_store(&search_running, false);
        pthread_join(search_thread, NULL);
    }

    if (timer_thread_live) {
        timer_thread_live = false;
        atomic_store(&timer_running, false);
        pthread_join(timer_thread, NULL);
    }
}

static void handle_quit(char *args) {
    handle_stop(args);

    if (tt_exists()) {
        free_tt();
    }

    free_sliding_attacks();
    
    running = false;
}

/* Function: encode_UCI
 * ---------------------
 * The `encode_UCI` function takes a move_t `move` and string `UCI_str`,
 * and populates `UCI_str` with the UCI-compatible representation of `move`.
 */
static void encode_UCI(move_t move, char *UCI_str) {
    move_flag flag = get_flag(move);
    char promo_str[2] = "";
    
    if (flag & PROMO_FLAG) {
        promo_str[0] = PIECE_ASCII[(flag & SPECIAL_FLAG) + KNIGHT] + ('a' - 'A');
        promo_str[1] = '\0';
    }
    
    sprintf(UCI_str, "%s%s%s",
            SQUARE_ASCII[get_from(move)],
            SQUARE_ASCII[get_to(move)],
            promo_str);
}

/* Function: decode_UCI
 * --------------------
 * The `decode_UCI` function takes a UCI-compatible string representation of a
 * move and returns its corresponding move_t representation. Assumed to be valid.
 */
static move_t decode_UCI(const board *b, const char *UCI_str) {
    int from_sq = get_square_from_ascii(UCI_str);
    int to_sq = get_square_from_ascii(UCI_str + 2);
    move_flag flag = QUIET;

    // PROMOTIONS
    char promo_char = UCI_str[4];
    if (promo_char) {
        promo_char += 'A' - 'a';
        move_flag special_flag;
        
        for (int i = 0; i < NUM_PROMOS; i++) {
            if (promo_char == PIECE_ASCII[i + KNIGHT]) {
                special_flag = i;
                break;
            }
        }

        flag |= (PROMO_FLAG | special_flag);
    }

    piece_t piece = piece_on(b, from_sq);

    // CAPTURES
    if (piece_on(b, to_sq) != NO_PIECE) {
        flag |= CAPTURE_FLAG;
    } else if (piece == PAWN &&
               b->ep_square != NO_EN_PASSANT &&
               to_sq == b->ep_square) {
        flag = EP_CAPTURE;
    }

    // CASTLING
    if (piece == KING) {
        if (!strcmp(UCI_str, "e1g1") || !strcmp(UCI_str, "e8g8")) {
            flag = KING_CASTLE;
        } else if (!strcmp(UCI_str, "e1c1") || !strcmp(UCI_str, "e8c8")) {
            flag = QUEEN_CASTLE;
        }
    }

    // DOUBLE PAWN PUSH
    if (piece == PAWN &&
        (1ULL << from_sq) & (RANK_2 | RANK_7) &&
        (1ULL << to_sq) & (RANK_4 | RANK_5)) {
        flag = DOUBLE_PAWN_PUSH;
    }
    
    return encode_move(from_sq, to_sq, flag);
}

/* The `search_helper` function is a thread function responsible for
 * running the search.
 *
 * It uses an iterative deepening algorithm until quitting the search
 * gets called.
 */
static void *search_helper(void *arg) {
    if (search_age == 127) {
        search_age = 0;
    } else {
        search_age ++;
    }

    search_context context = {
        .game_board = &game_board,
        .game_history = game_history,
        .n_searched = NULL,
        .age = game_board.fullmove_counter
    };
    
    move_t best_move = search(&context, MAX_DEPTH);

    char best_move_buf[6] = NULL_MOVE;

    if (best_move != NO_MOVE) {
        encode_UCI(best_move, best_move_buf);
    }
    
    printf("bestmove %s\n", best_move_buf);
    fflush(stdout);
    return NULL;
}

/* The `search_timer` function is a thread function responsible for
 * time management, calling off the search when a certain amount of
 * time has passed.
 *
 * Its argument is a pointer to the number of ms to wait.
 */
static void *search_timer(void *arg) {
    uint64_t elapsed_ms = 0;

    while (atomic_load(&timer_running) && elapsed_ms < timer_duration) {
        struct timespec interval_ts = {0, TIMER_INTERVAL * 1000000};
        int result = nanosleep(&interval_ts, NULL);
        if (result < 0) break;
        
        elapsed_ms += TIMER_INTERVAL;
    }

    atomic_store(&search_running, false);
    return NULL;
}
