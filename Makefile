# Compiler and flags
CXX = g++
CXXFLAGS = -Iinclude -Wall -Wextra -std=c++17 -g
LDFLAGS = -lcrypto -lz

# Directories
SRC_DIR = src
OBJ_DIR = build
INCLUDE_DIR = include
TEST_DIR = test

# Get all .cpp files in src/ and subdirectories
SRCS = $(shell find $(SRC_DIR) -name '*.cpp')

# Map source files to object files under build/
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Output executable
TARGET = $(TEST_DIR)/main.out

# Default rule
all: $(TARGET)

# Link object files to build executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

# Rule to compile .cpp to .o, ensuring the build dir structure exists
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	$(RM) -r $(OBJ_DIR)
	$(RM) -r $(TARGET)

# Run the program
run: all
	./$(TARGET)

.PHONY: all clean run
