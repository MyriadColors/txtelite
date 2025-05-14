CC = gcc
SRC = txtelite.c
TARGET = main

# Common compiler flags and linker flags
CFLAGS_COMMON = -std=c23
LDFLAGS_COMMON = -lm

# Debug specific flags (as per your command)
DEBUG_SPECIFIC_FLAGS = -Wall -Werror -Wextra

# Combine flags for different build types
# Debug build flags
CFLAGS_DEBUG = $(CFLAGS_COMMON) $(DEBUG_SPECIFIC_FLAGS)
# Release build flags (omitting -Wall -Werror -Wextra)
CFLAGS_RELEASE = $(CFLAGS_COMMON)

# Default target: build with debug flags
all: $(TARGET)

# Rule to build $(TARGET) with debug flags
# This is the default rule for building the target 'main'
$(TARGET): $(SRC)
	@echo "Compiling $(SRC) with debug flags..."
	$(CC) $(CFLAGS_DEBUG) $(SRC) -o $(TARGET) $(LDFLAGS_COMMON)
	@echo "Build complete: $(TARGET) (debug mode)"

# Target to build for release (no debug flags)
# This will re-compile $(TARGET) using release flags
release:
	@echo "Compiling $(SRC) for release..."
	$(CC) $(CFLAGS_RELEASE) $(SRC) -o $(TARGET) $(LDFLAGS_COMMON)
	@echo "Build complete: $(TARGET) (release mode)"

# Target to run the compiled program
# This depends on $(TARGET). If $(TARGET) is not present or outdated,
# it will be rebuilt using the default (debug) rule.
# If 'make release' was run previously and $(TARGET) is up-to-date, it runs that version.
run: $(TARGET)
	@echo "Running $(TARGET)..."
	.\$(TARGET)

# Target to clean build artifacts
clean:
	@echo "Cleaning up..."
	# rm -f is common in Makefiles and works with MinGW/MSYS environments on Windows
	rm -f $(TARGET)
	# Adding .exe just in case, though -o $(TARGET) should prevent it
	rm -f $(TARGET).exe
	@echo "Clean complete."

# Declare phony targets (targets that are not files)
.PHONY: all release run clean
