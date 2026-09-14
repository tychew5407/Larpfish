CC = gcc
CFLAGS = -g -Wall -Wextra -O3 -march=native -DNDEBUG -std=gnu99
LDLIBS = -lm

SRC_DIR = src
BUILD_DIR = build
TOOLS_DIR = tools

EXCLUDE_SRCS = $(SRC_DIR)/generate_magic.c $(SRC_DIR)/perft.c $(SRC_DIR)/test_search.c \
			  $(SRC_DIR)/generate_zobrist.c $(SRC_DIR)/generate_attack_tables.c \
			  $(SRC_DIR)/test_zobrist.c $(SRC_DIR)/main.c
COMMON_SRCS = $(filter-out $(EXCLUDE_SRCS), $(wildcard $(SRC_DIR)/*.c))
COMMON_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(COMMON_SRCS))

CHESS_SRCS = $(SRC_DIR)/main.c $(COMMON_SRCS) 
CHESS_OBJS = $(BUILD_DIR)/main.o $(COMMON_OBJS)

PERFT_SRCS = $(SRC_DIR)/perft.c $(COMMON_SRCS) 
PERFT_OBJS = $(BUILD_DIR)/perft.o $(COMMON_OBJS)

TEST_SEARCH_SRCS = $(SRC_DIR)/test_search.c $(COMMON_SRCS)
TEST_SEARCH_OBJS = $(BUILD_DIR)/test_search.o $(COMMON_OBJS)

ZOBRIST_SRCS = $(SRC_DIR)/generate_zobrist.c $(COMMON_SRCS)
ZOBRIST_OBJS = $(BUILD_DIR)/generate_zobrist.o $(COMMON_OBJS)

TEST_ZOBRIST_SRCS = $(SRC_DIR)/test_zobrist.c $(COMMON_SRCS)
TEST_ZOBRIST_OBJS = $(BUILD_DIR)/test_zobrist.o $(COMMON_OBJS)

CHESS_TARGET = Larpfish
PERFT_TARGET = $(TOOLS_DIR)/perft
TEST_SEARCH_TARGET = $(TOOLS_DIR)/test_search
ZOBRIST_TARGET = $(TOOLS_DIR)/generate_zobrist
TEST_ZOBRIST_TARGET = $(TOOLS_DIR)/test_zobrist

TARGETS = $(CHESS_TARGET) $(PERFT_TARGET) $(TEST_SEARCH_TARGET) $(ZOBRIST_TARGET) $(TEST_ZOBRIST_TARGET)

all: $(TARGETS)

$(CHESS_TARGET): $(CHESS_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(PERFT_TARGET): $(PERFT_OBJS) | $(TOOLS_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)
$(TEST_SEARCH_TARGET): $(TEST_SEARCH_OBJS) | $(TOOLS_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)
$(ZOBRIST_TARGET): $(ZOBRIST_OBJS) | $(TOOLS_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)
$(TEST_ZOBRIST_TARGET): $(TEST_ZOBRIST_OBJS) | $(TOOLS_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR) $(TOOLS_DIR):
	@mkdir -p $@

clean:
	rm -rf $(TARGETS) $(BUILD_DIR) $(TOOLS_DIR)

run: $(CHESS_TARGET)
	./$(CHESS_TARGET)

.PHONY: all clean run
