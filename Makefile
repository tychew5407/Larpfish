CC = gcc
CFLAGS = -g -Wall -Wextra -O3 -march=native -std=gnu99
SRC_DIR = src
BUILD_DIR = build
EXCLUDE = $(SRC_DIR)/generate_magic.c $(SRC_DIR)/perft.c $(SRC_DIR)/test_search.c \
		  $(SRC_DIR)/generate_zobrist.c
CHESS_SRCS = $(filter-out $(EXCLUDE), $(wildcard $(SRC_DIR)/*.c))
CHESS_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CHESS_SRCS))
MAGIC_SRCS = $(SRC_DIR)/generate_magic.c $(SRC_DIR)/bitboard.c
MAGIC_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(MAGIC_SRCS))
PERFT_SRCS = $(SRC_DIR)/perft.c $(SRC_DIR)/bitboard.c $(SRC_DIR)/board.c \
             $(SRC_DIR)/movegen.c $(SRC_DIR)/move_make.c \
             $(SRC_DIR)/fen.c
PERFT_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(PERFT_SRCS))
TEST_SEARCH_SRCS = $(SRC_DIR)/test_search.c $(SRC_DIR)/bitboard.c $(SRC_DIR)/board.c \
             $(SRC_DIR)/movegen.c $(SRC_DIR)/move_make.c $(SRC_DIR)/zobrist.c \
             $(SRC_DIR)/fen.c $(SRC_DIR)/evaluation.c $(SRC_DIR)/search.c
TEST_SEARCH_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(TEST_SEARCH_SRCS))
GENERATE_ZOBRIST_SRCS = $(SRC_DIR)/generate_zobrist.c $(SRC_DIR)/zobrist.c
GENERATE_ZOBRIST_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(GENERATE_ZOBRIST_SRCS))
CHESS_TARGET = chess
MAGIC_TARGET = generate_magic
PERFT_TARGET = perft
TEST_SEARCH_TARGET = test_search
GENERATE_ZOBRIST_TARGET = generate_zobrist
all: $(CHESS_TARGET) $(MAGIC_TARGET) $(PERFT_TARGET) $(TEST_SEARCH_TARGET) \
	 $(GENERATE_ZOBRIST_TARGET)
$(CHESS_TARGET): $(CHESS_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(MAGIC_TARGET): $(MAGIC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(PERFT_TARGET): $(PERFT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(TEST_SEARCH_TARGET): $(TEST_SEARCH_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(GENERATE_ZOBRIST_TARGET): $(GENERATE_ZOBRIST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf $(BUILD_DIR) $(CHESS_TARGET) $(MAGIC_TARGET) $(PERFT_TARGET) $(TEST_SEARCH_TARGET) \
		   $(GENERATE_ZOBRIST_TARGET)
run: $(CHESS_TARGET)
	./$(CHESS_TARGET)
.PHONY: all clean run
