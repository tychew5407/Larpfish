CC = gcc
CFLAGS = -g -Wall -Wextra -O3 -march=native -DNDEBUG -std=gnu99
SRC_DIR = src
BUILD_DIR = build
EXCLUDE = $(SRC_DIR)/generate_magic.c $(SRC_DIR)/perft.c $(SRC_DIR)/test_search.c \
		  $(SRC_DIR)/generate_zobrist.c $(SRC_DIR)/generate_attack_tables.c \
		  $(SRC_DIR)/test_zobrist.c
CHESS_SRCS = $(filter-out $(EXCLUDE), $(wildcard $(SRC_DIR)/*.c))
CHESS_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CHESS_SRCS))
PERFT_SRCS = $(SRC_DIR)/perft.c $(SRC_DIR)/bitboard.c $(SRC_DIR)/board.c \
             $(SRC_DIR)/movegen.c $(SRC_DIR)/zobrist_keys.c $(SRC_DIR)/move_make.c \
             $(SRC_DIR)/fen.c $(SRC_DIR)/zobrist.c $(SRC_DIR)/attack_tables.c
PERFT_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(PERFT_SRCS))
TEST_SEARCH_SRCS = $(SRC_DIR)/test_search.c $(SRC_DIR)/bitboard.c $(SRC_DIR)/board.c \
             $(SRC_DIR)/movegen.c $(SRC_DIR)/move_make.c $(SRC_DIR)/zobrist_keys.c \
			 $(SRC_DIR)/zobrist.c $(SRC_DIR)/fen.c $(SRC_DIR)/evaluation.c $(SRC_DIR)/search.c \
			 $(SRC_DIR)/transposition_table.c $(SRC_DIR)/attack_tables.c
TEST_SEARCH_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(TEST_SEARCH_SRCS))
ZOBRIST_SRCS = $(SRC_DIR)/generate_zobrist.c $(SRC_DIR)/zobrist_keys.c $(SRC_DIR)/zobrist.c
ZOBRIST_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(ZOBRIST_SRCS))
TEST_ZOBRIST_SRCS = $(SRC_DIR)/test_zobrist.c $(SRC_DIR)/zobrist_keys.c $(SRC_DIR)/zobrist.c \
					$(SRC_DIR)/board.c
TEST_ZOBRIST_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(TEST_ZOBRIST_SRCS))
CHESS_TARGET = chess
PERFT_TARGET = perft
TEST_SEARCH_TARGET = test_search
ZOBRIST_TARGET = generate_zobrist
TEST_ZOBRIST_TARGET = test_zobrist
all: $(CHESS_TARGET) $(MAGIC_TARGET) $(PERFT_TARGET) $(TEST_SEARCH_TARGET) \
	 $(ZOBRIST_TARGET) $(TEST_ZOBRIST_TARGET)
$(CHESS_TARGET): $(CHESS_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(PERFT_TARGET): $(PERFT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(TEST_SEARCH_TARGET): $(TEST_SEARCH_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(ZOBRIST_TARGET): $(ZOBRIST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(TEST_ZOBRIST_TARGET): $(TEST_ZOBRIST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf $(BUILD_DIR) $(CHESS_TARGET) $(PERFT_TARGET) $(TEST_SEARCH_TARGET) \
		   $(ZOBRIST_TARGET) $(TEST_ZOBRIST_TARGET)
run: $(CHESS_TARGET)
	./$(CHESS_TARGET)
.PHONY: all clean run
