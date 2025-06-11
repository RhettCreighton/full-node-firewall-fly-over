#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Sky Combat Comprehensive Test Suite
# Runs all unit and integration tests

set -e  # Exit on error

echo "================================================"
echo "    SKY COMBAT MULTIPLAYER TEST SUITE"
echo "================================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Track results
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test
run_test() {
    local test_name=$1
    local test_binary=$2
    
    echo -e "${YELLOW}Running: ${test_name}${NC}"
    echo "----------------------------------------"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [ -f "$test_binary" ]; then
        # Run test and capture exit code
        if $test_binary 2>/dev/null; then
            echo -e "${GREEN}✓ PASSED${NC}: ${test_name}\n"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            echo -e "${RED}✗ FAILED${NC}: ${test_name}\n"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    else
        echo -e "${RED}✗ NOT FOUND${NC}: ${test_binary}\n"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
}

# Build all tests
echo "Building test suite..."
echo "----------------------------------------"
if cmake . && make -j$(nproc) \
    test_aircraft_manager_unit \
    test_team_battles \
    test_powerups \
    test_multiplayer_complete; then
    echo -e "${GREEN}Build successful${NC}\n"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

# Run unit tests
echo "================================================"
echo "UNIT TESTS"
echo "================================================"
echo ""

run_test "Aircraft Manager Unit Tests" "./test_aircraft_manager_unit"

# Run integration tests
echo "================================================"
echo "INTEGRATION TESTS"
echo "================================================"
echo ""

run_test "Team Battle System" "./test_team_battles"
run_test "Power-up System" "./test_powerups"
run_test "Complete Multiplayer Integration" "./test_multiplayer_complete"

# Summary
echo "================================================"
echo "TEST SUMMARY"
echo "================================================"
echo ""
echo "Total Tests Run: ${TOTAL_TESTS}"
echo -e "Passed: ${GREEN}${PASSED_TESTS}${NC}"
echo -e "Failed: ${RED}${FAILED_TESTS}${NC}"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}✅ ALL TESTS PASSED!${NC}"
    echo "The Sky Combat multiplayer system is ready for integration."
    exit 0
else
    echo -e "${RED}❌ SOME TESTS FAILED!${NC}"
    echo "Please fix the failing tests before proceeding."
    exit 1
fi