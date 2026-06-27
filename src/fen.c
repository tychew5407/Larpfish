/* File: fen.c
 * --------------
 * For further comments, see "fen.h".
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "board.h"
#include "fen.h"

/* FUNCTION PROTOTYPES FOR HELPER FUNCTIONS */
static void parse_pieces(board *b, char *pieces_str);
static void parse_side(board *b, char *side_str);
static void parse_castling(board *b, char *castle_str);
static void parse_ep(board *b, char *ep_str);
static void parse_halfmove(board *b, char *halfmove_str);
static void parse_fullmove(board *b, char *fullmove_str);

static void encode_pieces(board *b, char **fen);
static void encode_side(board *b, char **fen);
static void encode_castling(board *b, char **fen);
static void encode_ep(board *b, char **fen);
static void encode_halfmove(board *b, char **fen);
static void encode_fullmove(board *b, char **fen);

// Get ASCII constants from `board.c`.
extern const char PIECE_ASCII[NUM_PIECES];
extern const char CASTLE_ASCII[NUM_CASTLES];

/* Store helper functions in arrays. */
static void (*parse_funcs[FEN_COMPONENTS])(board *, char *) = {
        parse_pieces,
        parse_side,
        parse_castling,
        parse_ep,
        parse_halfmove,
        parse_fullmove
};

static void (*encode_funcs[FEN_COMPONENTS])(board *, char **) = {
        encode_pieces,
        encode_side,
        encode_castling,
        encode_ep,
        encode_halfmove,
        encode_fullmove
};

/* HELPER FUNCTIONS FOR PARSE_FEN */
static void parse_pieces(board *b, char *pieces_str) {
    int square = SIDE_LEN * (SIDE_LEN - 1);

    for (size_t i = 0; i < strlen(pieces_str); i++) {
        if (pieces_str[i] == '/') {
            square -= 2 * SIDE_LEN;
            continue;
        }

        side cur_side;
        bitboard *cur_bb = get_bitboard_from_ascii(b, pieces_str[i], &cur_side);

        if (cur_bb) {
            set_bit(cur_bb, square);
            set_bit(&(b->occupied_bbs[cur_side]), square);
            square ++;
        } else {
            square += pieces_str[i] - '0';
        }
    }
}

static void parse_side(board *b, char *side_str) {
    b->play_side = (side_str[0] == 'b');
}

static void parse_castling(board *b, char *castle_str) {
    for (int i = 0; i < NUM_CASTLES; i++) {
        for (int j = 0; j < NUM_CASTLES; j++) {
            if (castle_str[i] == CASTLE_ASCII[j]) {
                b->castling |= (1 << j);
            }
        }
    }
}

static void parse_ep(board *b, char *ep_str) {
    if (ep_str[0] == '-') {
        b->ep_square = NO_EN_PASSANT;
    } else {
        b->ep_square = (ep_str[1] - '1') * SIDE_LEN + (ep_str[0] - 'a');
    }
}

static void parse_halfmove(board *b, char *halfmove_str) {
    b->halfmove_clock = atoi(halfmove_str);
}

static void parse_fullmove(board *b, char *fullmove_str) {
    b->fullmove_counter = atoi(fullmove_str);
}


/* HELPER FUNCTIONS FOR ENCODE_FEN */
static void encode_pieces(board *b, char **fen) {
    for (int row = SIDE_LEN - 1; row >= 0; row --) {
        int num_empty = 0;
        
        for (int col = 0; col < SIDE_LEN; col ++) {
            int square = col + (row * SIDE_LEN);
            piece_t piece;
            side side;
            
            bitboard *cur_bb = get_bitboard_from_square(b, square, &piece, &side);

            if (cur_bb) {
                if (num_empty > 0) {
                    char num_str[2] = {'0' + num_empty, '\0'};
                    strcat(*fen, num_str);
                    num_empty = 0;
                }

                char piece_str[2] = {PIECE_ASCII[piece] + ('a' - 'A') * side, '\0'};
                strcat(*fen, piece_str);
            } else {
                num_empty ++;
            }
        }

        if (num_empty > 0) {
            char num_str[2] = {'0' + num_empty, '\0'};
            strcat(*fen, num_str);
        }

        if (row > 0) {
            strcat(*fen, "/");
        }
    }
}

static void encode_side(board *b, char **fen) {
    char side_ch;

    if (b->play_side == WHITE) {
        side_ch = 'w';
    } else {
        side_ch = 'b';
    }

    char side_str[2] = {side_ch, '\0'};
    strcat(*fen, side_str);
}

static void encode_castling(board *b, char **fen) {
    if (b->castling == 0) {
        strcat(*fen, "-");
        return;
    }
    
    for (int i = NUM_CASTLES - 1; i >= 0; i--) {
        unsigned char mask = 1 << i;
        if ((b->castling & mask) > 0) {
            char castling_str[2] = {CASTLE_ASCII[i], '\0'};
            strcat(*fen, castling_str);
        }
    }
}

static void encode_ep(board *b, char **fen) {
    if (b->ep_square == NO_EN_PASSANT) {
        strcat(*fen, "-");
        return;
    }

    char ep_str[3] = {
        'a' + get_file(b->ep_square),
        '1' + get_rank(b->ep_square),
        '\0'
    };

    strcat(*fen, ep_str);
}

static void encode_halfmove(board *b, char **fen) {
    char move_str[10];
    sprintf(move_str, "%d", b->halfmove_clock);
    strcat(*fen, move_str);
}

static void encode_fullmove(board *b, char **fen) {
    char move_str[10];
    sprintf(move_str, "%d", b->fullmove_counter);
    strcat(*fen, move_str);
}


/* ---------------------------------- */

void parse_fen(board *b, char *fen) {
    char *cur = strtok(fen, " ");
    size_t parse_i = 0;

    while (cur) {
        parse_funcs[parse_i](b, cur);
        cur = strtok(NULL, " ");
        parse_i ++;
    }
}

void encode_fen(board *b, char **fen) {
    // Ensure empty string
    (*fen)[0] = '\0';

    for (int i = 0; i < FEN_COMPONENTS; i++) {
        encode_funcs[i](b, fen);

        if (i != FEN_COMPONENTS - 1) {
            strcat(*fen, " ");
        }
    }
}

void test_fen(char *filepath) {
    FILE *FEN_file = fopen(filepath, "r");

    if (!FEN_file) {
        printf("FAILURE: Invalid filepath!");
        return;
    }

    char fen_str[MAX_FEN_LEN];
    board b;
    int successes = 0;
    int tests = 0;

    while (fgets(fen_str, MAX_FEN_LEN, FEN_file) != NULL) {
        // Eliminate newline character
        fen_str[strcspn(fen_str, "\n")] = '\0';
        
        initialize_board(&b);
        
        char encoded_fen[MAX_FEN_LEN];
        strcpy(encoded_fen, fen_str);

        char *encoded_ptr = encoded_fen;

        parse_fen(&b, encoded_ptr);
        encode_fen(&b, &encoded_ptr);

        if (strcmp(fen_str, encoded_ptr)) {
            printf("FAILURE:\n  Expected: %s\n  Output: %s\n\n", fen_str, encoded_ptr);
        } else {
            successes ++;
        }

        tests ++;
    }
    
    printf("Overall: %d/%d cases passed.\n", successes, tests);
}
