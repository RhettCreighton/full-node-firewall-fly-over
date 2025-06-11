#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Memory leak checker for Sky Combat tests
# Requires: valgrind

set -e

echo "================================================"
echo "    SKY COMBAT MEMORY LEAK CHECKER"
echo "================================================"
echo ""

# Check if valgrind is installed
if ! command -v valgrind &> /dev/null; then
    echo "ERROR: valgrind is not installed"
    echo "Install with: sudo apt-get install valgrind (Ubuntu/Debian)"
    echo "         or: sudo dnf install valgrind (Fedora)"
    echo "         or: sudo zypper install valgrind (openSUSE)"
    exit 1
fi

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Track results
TOTAL_CHECKS=0
CLEAN_CHECKS=0
LEAK_CHECKS=0

# Function to check a binary for leaks
check_leaks() {
    local test_name=$1
    local test_binary=$2
    
    echo -e "${YELLOW}Checking: ${test_name}${NC}"
    echo "----------------------------------------"
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    
    if [ ! -f "$test_binary" ]; then
        echo -e "${RED}✗ NOT FOUND${NC}: ${test_binary}\n"
        LEAK_CHECKS=$((LEAK_CHECKS + 1))
        return
    fi
    
    # Run valgrind
    valgrind_output=$(mktemp)
    valgrind \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --verbose \
        --log-file="$valgrind_output" \
        "$test_binary" > /dev/null 2>&1
    
    # Check results
    if grep -q "All heap blocks were freed" "$valgrind_output"; then
        echo -e "${GREEN}✓ NO LEAKS${NC}: ${test_name}"
        CLEAN_CHECKS=$((CLEAN_CHECKS + 1))
    else
        echo -e "${RED}✗ MEMORY LEAK DETECTED${NC}: ${test_name}"
        LEAK_CHECKS=$((LEAK_CHECKS + 1))
        
        # Show leak summary
        echo "Leak Summary:"
        grep -A5 "LEAK SUMMARY" "$valgrind_output" || true
        echo "Full report saved to: ${valgrind_output}"
        echo ""
        return  # Don't delete the log file if there are leaks
    fi
    
    # Clean up if no leaks
    rm -f "$valgrind_output"
    echo ""
}

# Build tests first
echo "Building tests..."
if ! make -j$(nproc) test_aircraft_manager_unit 2>/dev/null; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi
echo ""

# Check each test
echo "================================================"
echo "MEMORY LEAK CHECKS"
echo "================================================"
echo ""

check_leaks "Aircraft Manager Unit Tests" "./test_aircraft_manager_unit"

# Add more tests as needed
# check_leaks "Other Test" "./other_test"

# Summary
echo "================================================"
echo "MEMORY CHECK SUMMARY"
echo "================================================"
echo ""
echo "Total Binaries Checked: ${TOTAL_CHECKS}"
echo -e "Clean (No Leaks): ${GREEN}${CLEAN_CHECKS}${NC}"
echo -e "Memory Leaks Found: ${RED}${LEAK_CHECKS}${NC}"
echo ""

if [ $LEAK_CHECKS -eq 0 ]; then
    echo -e "${GREEN}✅ NO MEMORY LEAKS DETECTED!${NC}"
    exit 0
else
    echo -e "${RED}❌ MEMORY LEAKS FOUND!${NC}"
    echo "Please fix the memory leaks before proceeding."
    exit 1
fi