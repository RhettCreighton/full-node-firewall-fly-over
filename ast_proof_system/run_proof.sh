#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Run complete AST+GDB mathematical proof on Sky Combat

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

echo "=== AST+GDB Mathematical Proof System ==="
echo "Proving Sky Combat cannot crash"
echo ""

# Check dependencies
if ! command -v clang &> /dev/null; then
    echo "ERROR: clang not found (needed for AST extraction)"
    exit 1
fi

if ! command -v gdb &> /dev/null; then
    echo "ERROR: gdb not found"
    exit 1
fi

if ! command -v python3 &> /dev/null; then
    echo "ERROR: python3 not found"
    exit 1
fi

# Check for z3 (optional but recommended)
if python3 -c "import z3" 2>/dev/null; then
    echo "✓ Z3 theorem prover available"
else
    echo "⚠ Z3 not found - symbolic execution limited"
    echo "  Install with: pip install z3-solver"
fi

# Files to verify
SOURCE_FILE="$PROJECT_ROOT/src/models/enemies.c"
BINARY_FILE="$PROJECT_ROOT/build/full-node-firewall-flyover"

# Check files exist
if [ ! -f "$SOURCE_FILE" ]; then
    echo "ERROR: Source file not found: $SOURCE_FILE"
    exit 1
fi

if [ ! -f "$BINARY_FILE" ]; then
    echo "ERROR: Binary not found: $BINARY_FILE"
    echo "Please build the project first"
    exit 1
fi

echo ""
echo "Source: $SOURCE_FILE"
echo "Binary: $BINARY_FILE"
echo ""

# Run verification
cd "$SCRIPT_DIR"

# Step 1: AST Analysis
echo "Step 1: Analyzing AST for dangerous operations..."
python3 ast_analyzer.py "$SOURCE_FILE" > ast_analysis.log 2>&1 || {
    echo "AST analysis failed. See ast_analysis.log"
    exit 1
}

# Show summary
grep -E "Found|PROVE:" ast_analysis.log || true

# Step 2: Complete verification
echo ""
echo "Step 2: Running complete verification..."
python3 complete_verifier.py "$SOURCE_FILE" "$BINARY_FILE" || {
    echo "Verification failed!"
    exit 1
}

echo ""
echo "=== VERIFICATION COMPLETE ==="
echo ""
echo "Generated files:"
echo "  - ast_analysis.log: AST analysis details"
echo "  - ast_generated_proof.gdb: GDB verification script"
echo "  - complete_mathematical_proof.md: Final proof document"
echo ""

# Show result
if [ -f complete_mathematical_proof.md ]; then
    echo "=== PROOF SUMMARY ==="
    grep -A5 "Mathematical Conclusion" complete_mathematical_proof.md | tail -n +3
fi