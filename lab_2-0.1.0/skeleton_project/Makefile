# Executable name
EXECUTABLE = elevator

# Compiler and flags
CC      = clang
CFLAGS  = -Wall -g -std=gnu11 -fsanitize=address -MMD -MP
LDFLAGS = -fsanitize=address

# Directories
SRC_DIR = source
OBJ_DIR = build

# Exclusion pattern for source files (e.g. exclude test files)
EXCLUDE = '*test*'

# Find all source files in SRC_DIR that do not match the exclusion pattern.
SOURCES := $(shell find $(SRC_DIR) -type f -name '*.c' -not -name $(EXCLUDE))
# Convert source file paths into object file paths
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Include auto-generated dependency files
-include $(OBJECTS:.o=.d)

.PHONY: all rebuild clean run

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

rebuild: clean all

clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLE)

run: $(EXECUTABLE)
	./$(EXECUTABLE)
