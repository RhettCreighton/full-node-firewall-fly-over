#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Benchmark script to compare build performance

set -e

echo "=== Build System Performance Benchmark ==="
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Create test directories
BASELINE_DIR="build_baseline"
OPTIMIZED_DIR="build_optimized"
ULTRA_DIR="build_ultra"

# Function to measure build time
measure_build() {
    local build_dir=$1
    local name=$2
    local cmake_args=$3
    
    echo -e "${YELLOW}Testing: $name${NC}"
    
    # Clean build
    rm -rf "$build_dir"
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    # Configure
    local config_start=$(date +%s.%N)
    cmake $cmake_args ../example &>/dev/null
    local config_end=$(date +%s.%N)
    local config_time=$(echo "$config_end - $config_start" | bc)
    
    # Build
    local build_start=$(date +%s.%N)
    make -j$(nproc) &>/dev/null
    local build_end=$(date +%s.%N)
    local build_time=$(echo "$build_end - $build_start" | bc)
    
    # Total time
    local total_time=$(echo "$config_time + $build_time" | bc)
    
    cd ..
    
    echo "  Configure: ${config_time}s"
    echo "  Build: ${build_time}s"
    echo -e "  ${GREEN}Total: ${total_time}s${NC}"
    echo ""
    
    echo "$name,$config_time,$build_time,$total_time"
}

# Results file
RESULTS="benchmark_results.csv"
echo "Test,Configure,Build,Total" > "$RESULTS"

# Test 1: Baseline (no optimizations)
measure_build "$BASELINE_DIR" "Baseline (No Optimizations)" \
    "-DENABLE_PROOF_SYSTEM=OFF -DENABLE_LTO=OFF" >> "$RESULTS"

# Test 2: Basic optimizations
measure_build "$OPTIMIZED_DIR" "Optimized (Basic)" \
    "-DENABLE_PROOF_SYSTEM=ON -DENABLE_LTO=ON" >> "$RESULTS"

# Test 3: Ultra optimizations
measure_build "$ULTRA_DIR" "Ultra (All Optimizations)" \
    "-DENABLE_ULTRA_PROOF=ON -DENABLE_DISTRIBUTED=ON -DENABLE_NATIVE_ARCH=ON -GNinja" >> "$RESULTS"

# Test incremental builds
echo -e "${YELLOW}Testing Incremental Builds${NC}"

# Make a small change
touch example/src/main.c

# Baseline incremental
cd "$BASELINE_DIR"
incr_start=$(date +%s.%N)
make -j$(nproc) &>/dev/null
incr_end=$(date +%s.%N)
baseline_incr=$(echo "$incr_end - $incr_start" | bc)
cd ..

# Ultra incremental
cd "$ULTRA_DIR"
incr_start=$(date +%s.%N)
ninja &>/dev/null
incr_end=$(date +%s.%N)
ultra_incr=$(echo "$incr_end - $incr_start" | bc)
cd ..

echo "  Baseline incremental: ${baseline_incr}s"
echo -e "  Ultra incremental: ${GREEN}${ultra_incr}s${NC}"
echo ""

# Calculate speedups
baseline_total=$(awk -F, 'NR==2 {print $4}' "$RESULTS")
optimized_total=$(awk -F, 'NR==3 {print $4}' "$RESULTS")
ultra_total=$(awk -F, 'NR==4 {print $4}' "$RESULTS")

opt_speedup=$(echo "scale=2; $baseline_total / $optimized_total" | bc)
ultra_speedup=$(echo "scale=2; $baseline_total / $ultra_total" | bc)
incr_speedup=$(echo "scale=2; $baseline_incr / $ultra_incr" | bc)

echo "=== Performance Summary ==="
echo ""
printf "%-20s %10s %10s %10s\n" "Configuration" "Time" "Speedup" "Relative"
printf "%-20s %10s %10s %10s\n" "--------------" "----" "-------" "--------"
printf "%-20s %10.2fs %10s %10s\n" "Baseline" "$baseline_total" "1.0x" "100%"
printf "%-20s %10.2fs %10.1fx %10.0f%%\n" "Optimized" "$optimized_total" "$opt_speedup" \
    $(echo "scale=0; 100 / $opt_speedup" | bc)
printf "%-20s %10.2fs %10.1fx %10.0f%%\n" "Ultra" "$ultra_total" "$ultra_speedup" \
    $(echo "scale=0; 100 / $ultra_speedup" | bc)
echo ""
echo "Incremental Build Speedup: ${incr_speedup}x"
echo ""

# Check available optimizations
echo "=== Available Optimizations ==="
echo -n "ccache: "
if command -v ccache &>/dev/null; then
    echo -e "${GREEN}✓ Available${NC}"
else
    echo -e "${RED}✗ Not found${NC} (install with: sudo apt install ccache)"
fi

echo -n "ninja: "
if command -v ninja &>/dev/null; then
    echo -e "${GREEN}✓ Available${NC}"
else
    echo -e "${RED}✗ Not found${NC} (install with: sudo apt install ninja-build)"
fi

echo -n "distcc: "
if command -v distcc &>/dev/null; then
    echo -e "${GREEN}✓ Available${NC}"
else
    echo -e "${YELLOW}○ Not found${NC} (optional - for distributed builds)"
fi

echo -n "clang: "
if command -v clang &>/dev/null; then
    echo -e "${GREEN}✓ Available${NC}"
else
    echo -e "${YELLOW}○ Not found${NC} (optional - for LLVM optimizations)"
fi

echo ""
echo "Benchmark complete! Results saved to: $RESULTS"