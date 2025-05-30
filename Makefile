CC = clang
SRC = txtelite.c
TARGET_BASENAME = main

# Common compiler flags
CFLAGS_COMMON = -std=c23
# LDFLAGS_COMMON will be for flags common to ALL OS, -lm is OS-specific
LDFLAGS_COMMON =

# OS-specific settings
EXEEXT =
RM = rm -f
LDFLAGS_OS = -lm # Default to Linux-like (e.g., for math library)
RUN_PREFIX = ./

ifeq ($(OS),Windows_NT)
    # Windows specific settings
    EXEEXT = .exe
    LDFLAGS_OS =
    RM = del /F /Q
    RUN_PREFIX = .\
endif

TARGET = $(TARGET_BASENAME)$(EXEEXT)

# Debug specific flags
DEBUG_SPECIFIC_FLAGS = -Wall -Werror -Wextra

# Combine flags for different build types
# Debug build flags
CFLAGS_DEBUG = $(CFLAGS_COMMON) $(DEBUG_SPECIFIC_FLAGS)
# Release build flags
CFLAGS_RELEASE = $(CFLAGS_COMMON)

# Default target: build with debug flags
all: $(TARGET)

# Rule to build $(TARGET) with debug flags
$(TARGET): $(SRC)
	@echo "Compiling $(SRC) with debug flags for $(OS)..."
	$(CC) $(CFLAGS_DEBUG) $(SRC) -o $(TARGET) $(LDFLAGS_COMMON) $(LDFLAGS_OS)
	@echo "Build complete: $(TARGET) (debug mode)"

# Target to build for release
release:
	@echo "Compiling $(SRC) for release for $(OS)..."
	$(CC) $(CFLAGS_RELEASE) $(SRC) -o $(TARGET) $(LDFLAGS_COMMON) $(LDFLAGS_OS)
	@echo "Build complete: $(TARGET) (release mode)"

# Target to run the compiled program
run: $(TARGET)
	@echo "Running $(TARGET)..."
	$(RUN_PREFIX)$(TARGET)

# Target to clean build artifacts
clean:
	@echo "Cleaning up..."
	$(RM) $(TARGET_BASENAME)$(EXEEXT)
	$(RM) $(TARGET_BASENAME) # Also try to remove without extension, just in case
	@echo "Clean complete."

# Declare phony targets
.PHONY: all release run clean
endif