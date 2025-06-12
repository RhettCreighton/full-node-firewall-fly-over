# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Makefile that REFUSES to compile unless GDB proves Firefox pixel will be drawn

CC = gcc
CFLAGS = -g -O0 -Wall
TARGET = firefox_app

# Source file
SRC = firefox_app.c

# The compiled binary (only created if proof passes)
BINARY = $(TARGET)

# Proof marker file
PROOF_MARKER = .gdb_pixel_proof_passed

# Default target
all: $(BINARY)

# The binary depends on passing the GDB proof
$(BINARY): $(PROOF_MARKER) $(SRC)
	@echo "✓ GDB proof passed - compiling $(TARGET)..."
	$(CC) $(CFLAGS) -o $(BINARY) $(SRC)
	@echo "✓ Compilation successful!"

# Generate the proof marker by running GDB verification
$(PROOF_MARKER): $(SRC)
	@echo "=== COMPILE-TIME GDB VERIFICATION ==="
	@echo "No compilation until GDB proves Firefox pixel draw!"
	@echo ""
	@chmod +x verify_final.sh
	@if ./verify_final.sh $(SRC); then \
		touch $(PROOF_MARKER); \
		echo "Proof marker created."; \
	else \
		echo ""; \
		echo "BUILD FAILED: GDB proof required!"; \
		echo "Your program must:"; \
		echo "  1. Have a firefox_draw_pixel() function"; \
		echo "  2. Actually call it during execution"; \
		echo "  3. Draw with Firefox colors (0xFF9500, etc)"; \
		echo ""; \
		exit 1; \
	fi

# Clean everything including proof
clean:
	rm -f $(BINARY) $(PROOF_MARKER) pre_compile_test *.gdb

# Force re-verification
verify: clean all

# Run the program (only works if it compiled)
run: $(BINARY)
	./$(BINARY)

# Special target to test with a bad program
test-bad:
	@echo "Testing with program that won't draw pixels..."
	@echo '#include <stdio.h>\nint main() { printf("No pixels!\\n"); return 0; }' > bad_program.c
	@if ./verify_pixel_draw.sh bad_program.c; then \
		echo "ERROR: Bad program passed (this shouldn't happen!)"; \
	else \
		echo "✓ Correctly rejected program without pixel drawing"; \
	fi
	@rm -f bad_program.c

.PHONY: all clean verify run test-bad

# Help message
help:
	@echo "Compile-Time GDB Proof Makefile"
	@echo "==============================="
	@echo "This Makefile REFUSES to compile unless GDB can prove"
	@echo "your program will draw a Firefox pixel!"
	@echo ""
	@echo "Commands:"
	@echo "  make        - Verify and compile (if proof passes)"
	@echo "  make clean  - Remove binary and proof"
	@echo "  make verify - Force re-verification"
	@echo "  make run    - Run the program"
	@echo "  make test-bad - Test rejection of non-pixel program"