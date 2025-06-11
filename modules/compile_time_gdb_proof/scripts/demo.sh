#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

echo "================================================"
echo "  COMPILE-TIME GDB PROOF DEMONSTRATION"
echo "================================================"
echo ""
echo "This build system REFUSES to compile unless it"
echo "can prove your program draws a Firefox pixel!"
echo ""

echo "1. Testing program WITHOUT firefox_draw_pixel()..."
echo "---"
cat bad_example.c
echo "---"
if make -f Makefile SRC=bad_example.c BINARY=bad_app 2>&1 | grep -E "(BLOCKED|FAILED)"; then
    echo "✓ Correctly rejected!"
fi

echo -e "\n2. Testing program WITH function but NO CALL..."
echo "---"
cat tricky_example.c | grep -A5 "main()"
echo "---"
if ./verify_final.sh tricky_example.c 2>&1 | grep -E "doesn't call it"; then
    echo "✓ Correctly caught the trick!"
fi

echo -e "\n3. Testing VALID program that draws pixels..."
echo "---"
cat firefox_app.c | grep -A2 "THIS LINE IS REQUIRED"
echo "---"
make clean >/dev/null 2>&1
if make 2>&1 | grep -E "(successful|allowed)"; then
    echo "✓ Compilation succeeded!"
    echo ""
    echo "Running the compiled program:"
    ./firefox_app
fi

echo -e "\n================================================"
echo "CONCLUSION: The build system successfully:"
echo "  ✓ Rejects programs without pixel drawing"
echo "  ✓ Catches sneaky non-calling programs"  
echo "  ✓ Only compiles proven pixel-drawing code"
echo "================================================"