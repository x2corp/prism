CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c11
LDFLAGS = -lm

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files
CORE_SRC = $(wildcard $(SRC_DIR)/core/*.c)
COMMON_SRC = $(wildcard $(SRC_DIR)/common/*.c)
LIB_SRC = $(wildcard $(SRC_DIR)/lib/*.c)
MAIN_SRC = $(SRC_DIR)/main.c

# Object files
CORE_OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CORE_SRC))
COMMON_OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(COMMON_SRC))
LIB_OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SRC))
MAIN_OBJ = $(BUILD_DIR)/main.o

# Target executable
TARGET = $(BIN_DIR)/prism

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR)/core
	@mkdir -p $(BUILD_DIR)/common
	@mkdir -p $(BUILD_DIR)/lib
	@mkdir -p $(BIN_DIR)

# Link object files to create executable
$(TARGET): $(CORE_OBJ) $(COMMON_OBJ) $(LIB_OBJ) $(MAIN_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compile core source files
$(BUILD_DIR)/core/%.o: $(SRC_DIR)/core/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Compile common source files
$(BUILD_DIR)/common/%.o: $(SRC_DIR)/common/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Compile lib source files
$(BUILD_DIR)/lib/%.o: $(SRC_DIR)/lib/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Compile main source file
$(BUILD_DIR)/main.o: $(MAIN_SRC)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Run an example
run: all
	$(TARGET) examples/main.prism

# Install to system (optional)
install: all
	install -m 755 $(TARGET) /usr/local/bin/

.PHONY: all directories clean run install