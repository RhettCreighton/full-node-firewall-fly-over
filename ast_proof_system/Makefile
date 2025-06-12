# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Simple Makefile for quick builds without CMake

.PHONY: all build test clean install analyze prove

# Python interpreter
PYTHON := python3

# Installation directory
PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin

all: build

build:
	@echo "=== Building AST Proof System ==="
	@mkdir -p build
	@cd build && cmake .. && make
	@echo "Build complete. Binaries in build/bin/"

test: build
	@echo "=== Testing AST Proof System ==="
	@cd build && make validate

analyze: build
	@echo "=== Running AST Analysis ==="
	@cd build && make analyze-ast

prove: build
	@echo "=== Running Deterministic Proof ==="
	@cd build && make prove

install: build
	@echo "=== Installing AST Proof System ==="
	@cd build && sudo make install
	@echo "Installed to $(PREFIX)"

clean:
	@echo "=== Cleaning ==="
	@rm -rf build
	@rm -f *.json *.md *.gdb *.c.bak
	@rm -rf .ast_proof_cache
	@rm -rf __pycache__

# Development targets
dev-setup:
	@echo "=== Setting up development environment ==="
	@$(PYTHON) -m pip install --user z3-solver
	@echo "Development environment ready"

quick-test:
	@echo "=== Quick test without CMake ==="
	@$(PYTHON) enhanced_ast_analyzer.py test_dangerous_code.c
	@$(PYTHON) determinism_analyzer.py test_determinism.c

# Integration with existing project
integrate:
	@echo "=== Integration Guide ==="
	@echo "Add to your CMakeLists.txt:"
	@echo "  add_subdirectory(ast_proof_system)"
	@echo "  add_ast_verification(your_target)"
	@echo ""
	@echo "Or include in your build:"
	@echo "  include(ast_proof_system/cmake/ASTProofSystem.cmake)"

help:
	@echo "AST GDB Deterministic Proving System"
	@echo ""
	@echo "Targets:"
	@echo "  make build    - Build the system"
	@echo "  make test     - Run validation tests"
	@echo "  make analyze  - Run AST analysis"
	@echo "  make prove    - Run deterministic proof"
	@echo "  make install  - Install system-wide"
	@echo "  make clean    - Clean build files"
	@echo ""
	@echo "Development:"
	@echo "  make dev-setup   - Install Python dependencies"
	@echo "  make quick-test  - Run tests without CMake"
	@echo "  make integrate   - Show integration guide"