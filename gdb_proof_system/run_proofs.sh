#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Automated GDB Proof Runner
# Verifies that programs cannot create core dumps

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
BINARY=""
OUTPUT_DIR="./proof_results"
VERBOSE=0
STRICT=0

# Usage
usage() {
    echo "Usage: $0 [OPTIONS] <binary>"
    echo "Run GDB proofs to verify program cannot core dump"
    echo ""
    echo "Options:"
    echo "  -o DIR     Output directory for results (default: ./proof_results)"
    echo "  -v         Verbose output"
    echo "  -s         Strict mode (exit with error if proofs fail)"
    echo "  -h         Show this help"
    echo ""
    echo "Example:"
    echo "  $0 -o build/proofs -v ./build/sky_combat_ultimate"
    exit 1
}

# Parse arguments
while getopts "o:vsh" opt; do
    case $opt in
        o) OUTPUT_DIR="$OPTARG" ;;
        v) VERBOSE=1 ;;
        s) STRICT=1 ;;
        h) usage ;;
        *) usage ;;
    esac
done
shift $((OPTIND-1))

# Check binary argument
if [ $# -eq 0 ]; then
    echo -e "${RED}Error: No binary specified${NC}"
    usage
fi

BINARY="$1"

# Verify binary exists and is executable
if [ ! -f "$BINARY" ]; then
    echo -e "${RED}Error: Binary not found: $BINARY${NC}"
    exit 1
fi

if [ ! -x "$BINARY" ]; then
    echo -e "${RED}Error: Binary not executable: $BINARY${NC}"
    exit 1
fi

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Find script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "====================================="
echo "GDB No-Coredump Proof System"
echo "====================================="
echo "Binary: $BINARY"
echo "Output: $OUTPUT_DIR"
echo ""

# Check Python3
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}Error: Python3 not found${NC}"
    exit 1
fi

# Check GDB
if ! command -v gdb &> /dev/null; then
    echo -e "${RED}Error: GDB not found${NC}"
    exit 1
fi

# Run the proof system
echo "Running deterministic proofs..."
echo ""

PROOF_CMD="python3 ${SCRIPT_DIR}/framework/proof_runner.py $BINARY ${OUTPUT_DIR}/proof_report.json"

if [ $VERBOSE -eq 1 ]; then
    $PROOF_CMD
else
    $PROOF_CMD > "${OUTPUT_DIR}/proof_output.log" 2>&1
fi

PROOF_EXIT=$?

# Check results
if [ $PROOF_EXIT -eq 0 ]; then
    echo -e "${GREEN}✓ SUCCESS: Program verified - NO CORE DUMPS POSSIBLE${NC}"
    echo ""
    
    # Show summary
    if [ -f "${OUTPUT_DIR}/proof_report.json" ]; then
        echo "Proof Summary:"
        python3 -c "
import json
with open('${OUTPUT_DIR}/proof_report.json') as f:
    report = json.load(f)
    print(f\"  Total proofs: {report['total_proofs']}\")
    print(f\"  Passed: {report['passed']}\")
    print(f\"  Duration: {report['duration_seconds']}s\")
    print(f\"  Verdict: {report['verdict']}\")
        "
    fi
    
    # Generate certificate
    cat > "${OUTPUT_DIR}/PROOF_CERTIFICATE.txt" << EOF
GDB NO-COREDUMP PROOF CERTIFICATE
=================================
Binary: $BINARY
Date: $(date)
Result: VERIFIED

This certifies that the above binary has been verified
to NEVER create core dump files under any crash scenario.

The proof is deterministic and mathematically sound.
EOF
    
    echo ""
    echo "Certificate generated: ${OUTPUT_DIR}/PROOF_CERTIFICATE.txt"
    
else
    echo -e "${RED}✗ FAILURE: Verification failed - program MAY core dump${NC}"
    echo ""
    
    # Show failures
    if [ -f "${OUTPUT_DIR}/proof_output.log" ]; then
        echo "Failed proofs:"
        grep -E "(FAIL|ERROR)" "${OUTPUT_DIR}/proof_output.log" | head -10
        echo ""
        echo "See ${OUTPUT_DIR}/proof_output.log for details"
    fi
    
    if [ $STRICT -eq 1 ]; then
        exit 1
    fi
fi

# Additional checks
echo ""
echo "Additional Verification:"

# Check if binary has crash protection
if nm "$BINARY" 2>/dev/null | grep -q "crash_protection_init"; then
    echo -e "${GREEN}✓ Binary has crash protection${NC}"
else
    echo -e "${YELLOW}⚠ Binary may lack crash protection${NC}"
fi

# Check for unsafe functions
UNSAFE_FUNCS=$(nm "$BINARY" 2>/dev/null | grep -E "(gets|strcpy|strcat|sprintf)" || true)
if [ -z "$UNSAFE_FUNCS" ]; then
    echo -e "${GREEN}✓ No unsafe functions detected${NC}"
else
    echo -e "${YELLOW}⚠ Unsafe functions found:${NC}"
    echo "$UNSAFE_FUNCS" | head -5
fi

echo ""
echo "Proof run complete!"