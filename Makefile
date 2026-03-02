TARGET = minesweeper

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lncurses

SRC_DIR = src
BUILD_DIR = build
SRC = $(SRC_DIR)/main.c
OBJ = $(BUILD_DIR)/main.o

all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
