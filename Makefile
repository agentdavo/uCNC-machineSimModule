# Makefile for CNC Machine Rendering Framework

# =============================================================================
#                                   VARIABLES
# =============================================================================

# Compiler
CC = gcc

# Directories
TGL_DIR = tinygl
STLIO_DIR = libstlio
STB_DIR = stb

# Compiler Flags
# -O3: Optimize for maximum speed
# -std=gnu99: Use GNU99 standard
# -I: Include directories for header files
CFLAGS = -O3 -std=gnu99 -Wall -g \
         -I./$(TGL_DIR)/include \
         -I./$(STLIO_DIR)/include \
         -I./$(STB_DIR) \
         -I.  # Include current directory for utils.h and other headers

# Linker Flags
# -lm: Link the math library
LDFLAGS = -lm -lmxml

# Source Files

# All .c files within TinyGL/src and libstlio/src
TGL_SRC = $(wildcard $(TGL_DIR)/src/*.c)
STLIO_SRC = $(wildcard $(STLIO_DIR)/src/*.c)

# Main Application Source
MAIN_SRC = main.c

# Additional Application Sources (refactored modules)
ADDITIONAL_SRC = utils.c actor.c assembly.c camera.c light.c config.c

# Object Files
# Replace .c with .o for object files
TGL_OBJ = $(TGL_SRC:.c=.o)
STLIO_OBJ = $(STLIO_SRC:.c=.o)
MAIN_OBJ = $(MAIN_SRC:.c=.o)
ADDITIONAL_OBJ = $(ADDITIONAL_SRC:.c=.o)

# Static Libraries
TGL_LIB = libtinygl.a
STLIO_LIB = libstlio.a

# Final Executable
TARGET = render_robot

# =============================================================================
#                                   TARGETS
# =============================================================================

# Default target
all: $(TARGET)

# Link the final executable
$(TARGET): $(MAIN_OBJ) $(ADDITIONAL_OBJ) $(TGL_LIB) $(STLIO_LIB)
	@echo "Linking $@..."
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) $(ADDITIONAL_OBJ) $(TGL_LIB) $(STLIO_LIB) $(LDFLAGS)
	@echo "Build complete: $@"

# Create TinyGL static library
$(TGL_LIB): $(TGL_OBJ)
	@echo "Creating static library $@..."
	ar rcs $@ $^

# Create libstlio static library
$(STLIO_LIB): $(STLIO_OBJ)
	@echo "Creating static library $@..."
	ar rcs $@ $^

# Compile TinyGL source files into object files
$(TGL_DIR)/src/%.o: $(TGL_DIR)/src/%.c
	@echo "Compiling TinyGL: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile libstlio source files into object files
$(STLIO_DIR)/src/%.o: $(STLIO_DIR)/src/%.c
	@echo "Compiling libstlio: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile additional application source files into object files
%.o: %.c
	@echo "Compiling application source: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile main application source file into object file
# Explicitly specify dependency on stb_image_write.h
$(MAIN_OBJ): $(MAIN_SRC) $(STB_DIR)/stb_image_write.h
	@echo "Compiling main application: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TGL_DIR)/src/*.o
	rm -f $(STLIO_DIR)/src/*.o
	rm -f $(TGL_LIB) $(STLIO_LIB)
	rm -f $(MAIN_OBJ)
	rm -f $(ADDITIONAL_OBJ)
	rm -f $(TARGET)
	@echo "Clean complete."

# =============================================================================
#                                   PHONY TARGETS
# =============================================================================

.PHONY: all clean