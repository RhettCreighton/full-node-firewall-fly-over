#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Complete test suite for AST GDB proving system
set -e

echo "=== AST GDB Proving System - Complete Test Suite ==="
echo

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test results
PASSED=0
FAILED=0

# Function to run a test
run_test() {
    local test_name="$1"
    local command="$2"
    
    echo -n "Testing $test_name... "
    
    if eval "$command" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ PASSED${NC}"
        ((PASSED++))
    else
        echo -e "${RED}✗ FAILED${NC}"
        ((FAILED++))
        echo "  Command: $command"
    fi
}

# Function to check if file exists
check_file() {
    local file="$1"
    local description="$2"
    
    echo -n "Checking $description... "
    
    if [ -f "$file" ]; then
        echo -e "${GREEN}✓ EXISTS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}✗ MISSING${NC}"
        ((FAILED++))
    fi
}

echo "1. Checking dependencies..."
echo "=========================="

# Check for required tools
run_test "Python 3" "python3 --version"
run_test "Clang" "clang --version"
run_test "GDB" "gdb --version"

echo
echo "2. Testing enhanced AST analyzer..."
echo "==================================="

# Test enhanced analyzer on test file
run_test "Enhanced analyzer syntax" "python3 enhanced_ast_analyzer.py test_dangerous_code.c"

# Check if report is generated
check_file "ast_safety_report.md" "Safety report generation"

# Test on actual game code
if [ -f "../src/models/enemies.c" ]; then
    run_test "Analyze game code" "python3 enhanced_ast_analyzer.py ../src/models/enemies.c"
fi

echo
echo "3. Testing validation suite..."
echo "=============================="

# Run validation
run_test "Validation suite" "python3 validate_analyzer.py enhanced_ast_analyzer.py test_dangerous_code.c"

# Check validation report
check_file "validation_report.json" "Validation report"

echo
echo "4. Testing incremental verifier..."
echo "=================================="

# Test incremental verification
run_test "Incremental verify" "python3 incremental_verifier.py . --report incremental_report.md"

# Check cache creation
check_file ".ast_proof_cache/proof_cache.json" "Proof cache"

# Test cache effectiveness
echo -e "${YELLOW}Testing cache performance...${NC}"
START_TIME=$(date +%s.%N)
python3 incremental_verifier.py . > /dev/null 2>&1
END_TIME=$(date +%s.%N)
FIRST_RUN=$(echo "$END_TIME - $START_TIME" | bc)

START_TIME=$(date +%s.%N)
python3 incremental_verifier.py . > /dev/null 2>&1
END_TIME=$(date +%s.%N)
CACHED_RUN=$(echo "$END_TIME - $START_TIME" | bc)

echo "  First run: ${FIRST_RUN}s"
echo "  Cached run: ${CACHED_RUN}s"

# Check if cached run is faster
if (( $(echo "$CACHED_RUN < $FIRST_RUN" | bc -l) )); then
    echo -e "  ${GREEN}✓ Cache is working (faster on second run)${NC}"
    ((PASSED++))
else
    echo -e "  ${RED}✗ Cache not effective${NC}"
    ((FAILED++))
fi

echo
echo "5. Testing CI integration..."
echo "============================"

# Test CI verification
run_test "CI integration" "python3 ci_integration.py --ci local --verify-all"

# Check CI outputs
check_file "proof_results.json" "CI results JSON"
check_file "safety_report.html" "HTML report"
check_file "junit_report.xml" "JUnit report"

# Test workflow generation
run_test "GitHub workflow gen" "python3 ci_integration.py --generate-workflow github"
run_test "GitLab CI gen" "python3 ci_integration.py --generate-workflow gitlab"

echo
echo "6. Testing complete verification..."
echo "==================================="

# Run complete verifier if exists
if [ -f "complete_verifier.py" ]; then
    run_test "Complete verification" "python3 complete_verifier.py test_dangerous_code.c /usr/bin/true"
fi

echo
echo "7. Testing AST parser accuracy..."
echo "================================="

# Create a simple test to verify AST parsing
cat > ast_test.c << 'EOF'
int divide(int a, int b) {
    return a / b;  // Should be detected
}

int safe_divide(int a, int b) {
    if (b != 0) {
        return a / b;  // Should NOT be detected
    }
    return 0;
}
EOF

echo -n "Testing AST detection accuracy... "
OUTPUT=$(python3 enhanced_ast_analyzer.py ast_test.c 2>&1)

if echo "$OUTPUT" | grep -q "divide.*unsafe" && ! echo "$OUTPUT" | grep -q "safe_divide.*unsafe"; then
    echo -e "${GREEN}✓ Accurate detection${NC}"
    ((PASSED++))
else
    echo -e "${RED}✗ Detection issues${NC}"
    ((FAILED++))
fi

rm -f ast_test.c

echo
echo "8. Performance benchmark..."
echo "=========================="

# Create larger test file
echo -n "Creating benchmark file... "
cat > benchmark.c << 'EOF'
#include <math.h>
EOF

# Add 1000 functions with various operations
for i in {1..1000}; do
    cat >> benchmark.c << EOF
float func_$i(float a, float b) {
    float result = a / b;  // Dangerous
    result = sqrt(result); // Potentially dangerous
    return result;
}
EOF
done

echo "done (1000 functions)"

echo -n "Benchmarking analyzer... "
START_TIME=$(date +%s.%N)
python3 enhanced_ast_analyzer.py benchmark.c > /dev/null 2>&1
END_TIME=$(date +%s.%N)
ANALYZE_TIME=$(echo "$END_TIME - $START_TIME" | bc)

echo "completed in ${ANALYZE_TIME}s"

# Check if performance is reasonable (< 10s for 1000 functions)
if (( $(echo "$ANALYZE_TIME < 10" | bc -l) )); then
    echo -e "  ${GREEN}✓ Good performance${NC}"
    ((PASSED++))
else
    echo -e "  ${YELLOW}⚠ Slow performance${NC}"
fi

rm -f benchmark.c

echo
echo "=================================="
echo "Test Summary"
echo "=================================="
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}✓ All tests passed! The AST GDB proving system is rock solid.${NC}"
    exit 0
else
    echo -e "\n${RED}✗ Some tests failed. Please fix the issues above.${NC}"
    exit 1
fi